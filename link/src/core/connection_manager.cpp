/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "connection_manager.h"
#include "plugin_directory.h"
#include "plugin.h"
#include "log.h"
#include "common/message_stream.h"
#include "message_queue.h"

#include "base/core/assert.h"
#include "base/core/str.h"
#include "base/core/time_utils.h"
#include "base/threading/thread.h"
#include <atomic>

#include "common/protobuf_stream.h"
#include "protocol/handshake.pb.h"

const Base::LogChannel kConnectLog("connect");

#define LOG_INFO(...) LINK_LOG(kConnectLog, kLogInfo, __VA_ARGS__)
#define LOG_ERROR(...) LINK_LOG(kConnectLog, kLogError, __VA_ARGS__)

namespace Link {

bool operator==(const ConnectionListener::Connection &rhs,
                const ConnectionListener::Connection &lhs) {
  return rhs.handle == lhs.handle && rhs.endpoint == lhs.endpoint;
}

const char *ToString(ConnectionState state) {
  switch(state) {
  case kDisconnected:
    return "kDisconnected";
  case kConnecting:
    return "kConnecting";
  case kHandshake:
    return "kHandshake";
  case kEstablished:
    return "kEstablished";
  case kError:
    return "kError";
  }
  return "UNKNOWN";
}

static void *kListenSocketId = (void *)-1;
static void *kInterruptId = (void *)-2;
const streamsize kTcpConnectionBytes = 10 * 1024; // todo
const streamsize kInProcessConnectionBytes = 10 * 1024;
static const u16 InvalidIndex = -1;

//------------------------------------------------------------------------------

namespace {
void Init(TCPConnectionData &obj, u16 capacity) {
  obj.count = 0;
  obj.capacity = capacity;
  obj.connection = new RemoteConnection[capacity];
  memset(obj.connection, 0, sizeof(RemoteConnection) * capacity);
  obj.epoll = Base::EpollCreate(capacity);
  obj.listen_socket = Base::Socket::Tcp::Open();
}
void Release(TCPConnectionData &obj) {
  delete[] obj.connection;
  Base::EpollDestroy(obj.epoll);
  Base::Socket::Close(obj.listen_socket);
}
u16 AddRemoteConnection(TCPConnectionData &obj) {
  BASE_ASSERT(obj.count < obj.capacity, "can't create more remote connections");
  u16 index = obj.count;
  obj.count++;
  return index; // RemoteConnection& conn = m_tcp.connection[index];
}
u16 RemoveRemoteConnection(TCPConnectionData &obj, u16 remove_index) {
  BASE_ASSERT(remove_index < obj.count, "index out of bounds");
  u16 swapped = InvalidIndex;
  u16 last_index = obj.count - 1;
  if(remove_index != last_index) {
    // move last element into freed slot.
    obj.connection[remove_index] = obj.connection[last_index];
    swapped = last_index;
  }
  --obj.count;
  return swapped;
}
bool CanClose(RemoteConnection &conn) {
  return conn.out_messages->Empty() && conn.in_messages->Empty();
}
};

namespace {
void Init(InProcConnectionData &obj, u16 capacity) {
  obj.count = 0;
  obj.capacity = capacity;
  obj.connection = new LocalConnection[capacity];
  memset(obj.connection, 0, sizeof(LocalConnection) * capacity);
}
void Release(InProcConnectionData &obj) { delete[] obj.connection; }
u16 AddLocalConnection(InProcConnectionData &obj) {
  BASE_ASSERT(obj.count < obj.capacity,
              "can't create more local connections %d/%d", obj.count,
              obj.capacity);
  u16 index = obj.count;
  obj.count++;
  return index; // LocalConnection& conn = m_inproc.connection[index];
}
u16 RemoveLocalConnection(InProcConnectionData &obj, u16 remove_index) {
  BASE_ASSERT(remove_index < obj.count, "index out of bounds");

  u16 swapped = InvalidIndex;
  u16 last_index = obj.count - 1;
  if(remove_index != last_index) {
    // move last element into freed slot.
    obj.connection[remove_index] = obj.connection[last_index];
    swapped = last_index;
  }
  --obj.count;
  return swapped;
}
bool CanClose(LocalConnection &conn) { return conn.in_messages->Empty(); }
};

//------------------------------------------------------------------------------

struct ConnectionVfptr {
  void (*DestroyConnection)(ConnectionManager &obj, u16 index);
  bool (*CanClose)(const ConnectionManager &obj, u16 index);
  bool (*Write)(ConnectionManager &obj, u16 index, const void *data,
                streamsize nbytes);
  unsigned int (*TotalMessageSize)(const ConnectionManager &obj, u16 index);
  unsigned int (*NextRecvAvailable)(const ConnectionManager &obj, u16 index);
  Result (*Read)(ConnectionManager &obj, u16 index, void *bytes,
                 streamsize nbytes, streamsize *received);
};

namespace {
void tcp_destroy_connection(ConnectionManager &obj, u16 index) {
  obj.DestroyRemoteConnection(index);
}
bool tcp_can_close(const ConnectionManager &obj, u16 index) {
  return CanClose(obj.m_tcp.connection[index]);
}
bool tcp_write(ConnectionManager &obj, u16 index, const void *data,
               streamsize nbytes) {
  return obj.m_tcp.connection[index].out_messages->Write(data, nbytes);
}
unsigned int tcp_total_message_size(const ConnectionManager &obj, u16 index) {
  return obj.m_tcp.connection[index].in_messages->TotalMessageSize();
}
unsigned int tcp_next_message_size(const ConnectionManager &obj, u16 index) {
  return obj.m_tcp.connection[index].in_messages->NextMessageSize();
}
Result tcp_read(ConnectionManager &obj, u16 index, void *bytes,
                streamsize nbytes, streamsize *received) {
  return obj.m_tcp.connection[index].in_messages->Read(bytes, nbytes, received);
}
void inproc_destroy_connection(ConnectionManager &obj, u16 index) {
  obj.DestroyLocalConnection(index);
}
bool inproc_can_close(const ConnectionManager &obj, u16 index) {
  return CanClose(obj.m_inproc.connection[index]);
}
bool inproc_write(ConnectionManager &obj, u16 index, const void *data,
                  streamsize nbytes) {
  return obj.m_inproc.connection[index].out_messages->Write(data, nbytes);
}
unsigned int inproc_total_message_size(const ConnectionManager &obj,
                                       u16 index) {
  return obj.m_inproc.connection[index].in_messages->TotalMessageSize();
}
unsigned int inproc_next_message_size(const ConnectionManager &obj, u16 index) {
  return obj.m_inproc.connection[index].in_messages->NextMessageSize();
}
Result inproc_read(ConnectionManager &obj, u16 index, void *bytes,
                   streamsize nbytes, streamsize *received) {
  return obj.m_inproc.connection[index].in_messages->Read(bytes, nbytes,
                                                          received);
}
}

static ConnectionVfptr kTcpVfptr{
    tcp_destroy_connection, tcp_can_close,         tcp_write,
    tcp_total_message_size, tcp_next_message_size, tcp_read};
static ConnectionVfptr kInProcVfptr{
    inproc_destroy_connection, inproc_can_close,         inproc_write,
    inproc_total_message_size, inproc_next_message_size, inproc_read};

//------------------------------------------------------------------------------

void Connection::Init(ConnectionHandle _handle, PluginHandle _source,
                      PluginHandle _dest, ConnectionListener *_listener,
                      u16 _detail_index, u16 _flags) {
  vfptr = _flags & Connection::kLocal ? &kInProcVfptr : &kTcpVfptr;
  handle = _handle;
  state = ConnectionState::kDisconnected;
  endpoint = _dest;
  listener = _listener;
  detail_index = _detail_index;
  flags = _flags;
  time = Base::Time::GetTimeMs();
  LOG_INFO("conn(%p) state change %s. %s, %s.", handle,
           ToString(ConnectionState::kDisconnected),
           flags & Connection::kLocal ? "local" : "remote",
           flags & Connection::kOutgoing ? "outgoing" : "incoming");
}

void Connection::SetState(ConnectionState _state) {
  if(state == _state) {
    LOG_ERROR("re-setting the same state for connection %p. state: %s", handle,
              ToString(state));
  }
  ConnectionState old_state = state;
  state = _state;

  LOG_INFO("conn(%p) state change %s -> %s. %s, %s.", handle,
           ToString(old_state), ToString(state),
           flags & Connection::kLocal ? "local" : "remote",
           flags & Connection::kOutgoing ? "outgoing" : "incoming");

  if(listener) {
    ConnectionListener::Connection data;
    data.handle = handle;
    data.endpoint = endpoint;
    data.outgoing = flags & Connection::kOutgoing;
    listener->OnStateChange(data, old_state, state);
  }
}

void LocalConnection::Init(ConnectionManager *_remote,
                           ConnectionHandle _remote_handle,
                           streamsize _buffer_size) {
  remote_manager = _remote;
  remote_handle = _remote_handle;
  in_messages = new MessageQueue(_buffer_size);
}

//------------------------------------------------------------------------------

ConnectionManager::ConnectionManager(PluginDirectory &plugins,
                                     u32 max_num_connections)
    : m_plugin_dir(plugins), m_owner(0), m_handle_generator(0),
      m_max_connections(max_num_connections), m_connection_count(0) {
  Init(m_tcp, max_num_connections);
  Init(m_inproc, max_num_connections);
  m_connection_list = new ConnectionHandle[m_max_connections];
  m_data = new Connection[m_max_connections];
  memset(m_data, 0, sizeof(Connection) * max_num_connections);
  m_mutex.Initialize();
  BASE_ASSERT(sizeof(m_handle_generator) == sizeof(ConnectionHandle),
              "s_handle must match ConnectionHandle size");
}

ConnectionManager::~ConnectionManager() {
  Release(m_tcp);
  Release(m_inproc);
  m_mutex.Terminate();
  delete[] m_connection_list;
  delete[] m_data;
}

void ConnectionManager::Initialize(PluginHandle owner) { m_owner = owner; }

bool ConnectionManager::CreateListenSocket(u16 &port) {
  Base::MutexAutoLock lock(m_mutex);
  bool result = Base::Socket::Tcp::Listen(m_tcp.listen_socket, &port);
  if(result) {
    LOG_INFO("(epoll) adding watch for sock(%d). data %d.", m_tcp.listen_socket,
             kListenSocketId);
    result = Base::EpollAdd(m_tcp.epoll, m_tcp.listen_socket, kListenSocketId,
                            Base::Epoll::OP_ALL);
    if(!result) {
      LOG_ERROR("failed to add listen socket to epoll");
    }
  }
  return result;
}

ConnectionHandle ConnectionManager::GenerateHandle() {
  s64 handle = 0;
  do {
    static std::atomic_int s_handle(0);
    handle = ++s_handle; // m_handle_generator;
  } while(FindIndex(reinterpret_cast<ConnectionHandle>(handle)) !=
          InvalidIndex);
  return reinterpret_cast<ConnectionHandle>(handle);
}

ConnectionHandle
ConnectionManager::OpenLocal(ConnectionManager &rhs,
                             Link::ConnectionListener *listener_local,
                             ConnectionListener *listener_remote) {
  Base::MutexAutoLock lock(m_mutex);
  Base::MutexAutoLock rhs_lock(rhs.m_mutex);

  PluginHandle local_id = m_owner;
  PluginHandle remote_id = rhs.m_owner;

  // get local and remote plugin info.
  PluginInfo local_endpoint, remote_endpoint;
  if(!m_plugin_dir.GetInfo(local_id, local_endpoint)) {
    LOG_ERROR("(connect) %p local endpoint not found for %p", m_owner,
              local_id);
    return 0;
  }
  if(!m_plugin_dir.GetInfo(remote_id, remote_endpoint)) {
    LOG_ERROR("(connect) %p remote endpoint not found for %p", m_owner,
              remote_id);
    return 0;
  }

  LOG_INFO("(connect) %p opening connection %p %s:%d(%d) -> %p %s:%d(%d)",
           m_owner, local_id, local_endpoint.hostname, local_endpoint.port,
           local_endpoint.pid, remote_id, remote_endpoint.hostname,
           remote_endpoint.port, remote_endpoint.pid);

  ConnectionHandle handle = GenerateHandle();
  ConnectionHandle rhs_handle = rhs.GenerateHandle();

  BASE_ASSERT(FindIndex(handle) == InvalidIndex, "handle already taken %p",
              handle);
  BASE_ASSERT(rhs.FindIndex(rhs_handle) == InvalidIndex,
              "handle already taken %p", rhs_handle);

  u16 index = AddConnection(handle);
  if(index == InvalidIndex) {
    LOG_ERROR("(connect) %p could not add connection. limit exceeded (%d/%d) "
              "or handle already exists",
              m_owner, m_connection_count, m_max_connections);
    return 0;
  }

  u16 rhs_index = rhs.AddConnection(rhs_handle);
  if(rhs_index == InvalidIndex) {
    LOG_ERROR("(connect) %p could not add remote connection. limit exceeded "
              "(%d/%d) or handle already exists",
              rhs.m_owner, rhs.m_connection_count, rhs.m_max_connections);
    RemoveConnection(index);
    return 0;
  }

  // init local endpoint.
  Connection &common = m_data[index];
  common.Init(handle, local_id, remote_id, listener_local,
              AddLocalConnection(m_inproc),
              Connection::kLocal | Connection::kOutgoing);
  LocalConnection &local_data = m_inproc.connection[common.detail_index];
  local_data.Init(&rhs, rhs_handle, kInProcessConnectionBytes);

  // init remote endpoint.
  Connection &rhs_common = rhs.m_data[rhs_index];
  rhs_common.Init(rhs_handle, remote_id, local_id, listener_remote,
                  AddLocalConnection(rhs.m_inproc), Connection::kLocal);
  LocalConnection &rhs_local_data =
      rhs.m_inproc.connection[rhs_common.detail_index];
  rhs_local_data.Init(this, handle, kInProcessConnectionBytes);

  common.SetState(ConnectionState::kConnecting);
  rhs_common.SetState(ConnectionState::kConnecting);

  // bind message queues.
  local_data.out_messages = rhs_local_data.in_messages;
  rhs_local_data.out_messages = local_data.in_messages;

  common.SetState(ConnectionState::kEstablished);
  rhs_common.SetState(ConnectionState::kEstablished);

  BASE_ASSERT(m_owner == local_id);
  BASE_ASSERT(rhs.m_owner == remote_id);

  LOG_INFO(
      "(connect) %p opened connection %p %s:%d(%d) -> %p %s:%d(%d) (%p -> %p)",
      m_owner, local_id, local_endpoint.hostname, local_endpoint.port,
      local_endpoint.pid, remote_id, remote_endpoint.hostname,
      remote_endpoint.port, remote_endpoint.pid, common.handle,
      rhs_common.handle);

  LOG_INFO(
      "(connect) %p opened connection %p %s:%d(%d) -> %p %s:%d(%d) (%p -> %p)",
      rhs.m_owner, remote_id, remote_endpoint.hostname, remote_endpoint.port,
      remote_endpoint.pid, local_id, local_endpoint.hostname,
      local_endpoint.port, local_endpoint.pid, rhs_common.handle,
      common.handle);

  return handle;
}

ConnectionHandle
ConnectionManager::OpenRemote(ConnectionListener *listener_local,
                              PluginHandle local_id, PluginHandle remote_id) {
  BASE_ASSERT(local_id != remote_id, "can't remote connect to self (%p)",
              local_id);

  Base::MutexAutoLock lock(m_mutex);

  PluginInfo local_endpoint, remote_endpoint;
  // find both plugins in dictionary.
  if(!m_plugin_dir.GetInfo(local_id, local_endpoint)) {
    LOG_ERROR("(connect) local endpoint not found for %d", local_id);
    return 0;
  }
  if(!m_plugin_dir.GetInfo(remote_id, remote_endpoint)) {
    LOG_ERROR("(connect) remote endpoint not found for %d", remote_id);
    return 0;
  }

  LOG_INFO("(connect) opening connection %p %s:%d(%d) -> %p %s:%d(%d)",
           local_id, local_endpoint.hostname, local_endpoint.port,
           local_endpoint.pid, remote_id, remote_endpoint.hostname,
           remote_endpoint.port, remote_endpoint.pid);

  Base::Socket::Handle socket = Base::Socket::Tcp::Open();
  if(Base::Socket::InvalidHandle == socket) {
    LINK_WARN("(connect) failed to create a socket for tcp connection");
    return 0;
  }

  ConnectionHandle handle = GenerateHandle();
  BASE_ASSERT(FindIndex(handle) == InvalidIndex, "handle already taken %p",
              handle);

  Base::Url url(remote_endpoint.hostname, remote_endpoint.port);

  LOG_INFO(
      "open remote to %d.%d.%d.%d:%d sock(%d). conn(%p) plug(%p) -> plug(%p)",
      url.GetAddress().GetA(), url.GetAddress().GetB(), url.GetAddress().GetC(),
      url.GetAddress().GetD(), url.GetPort(), socket, handle, local_id,
      remote_id);

  if(Base::Socket::Tcp::kFailed == Base::Socket::Tcp::Connect(socket, url)) {
    LINK_WARN("(connect) failed to connect socket for tcp connection");
    Base::Socket::Close(socket);
    return 0;
  }

  u16 index = AddConnection(handle);
  if(index == InvalidIndex) {
    LINK_WARN("(connect) failed to add a remote connection - add failed");
    Base::Socket::Close(socket);
    return 0;
  }

  u16 detail_index = AddRemoteConnection(m_tcp);

  Connection &common = m_data[index];
  common.Init(handle, local_id, remote_id, listener_local, detail_index,
              Connection::kOutgoing);
  common.SetState(ConnectionState::kConnecting);

  RemoteConnection &detail = m_tcp.connection[detail_index];
  detail = {};
  detail.socket = socket;
  detail.in_messages = new MessageInStream(kTcpConnectionBytes);
  detail.out_messages = new MessageOutStream(kTcpConnectionBytes);

  LOG_INFO("(epoll) adding watch for sock(%d). data %d.", socket, index);
  bool epoll_added = Base::EpollAdd(
      m_tcp.epoll, socket, reinterpret_cast<void *>(index),
      Base::Epoll::OP_READ | Base::Epoll::OP_WRITE | Base::Epoll::OP_ERROR);
  BASE_ASSERT(epoll_added,
              "(connect) failed to add epoll operation for sock(%d)", socket);

  LOG_INFO("(connect)	opened connection %p %s:%d(%d) -> %p %s:%d(%d) (%p -> "
           "%p). on sock(%d)",
           local_id, local_endpoint.hostname, local_endpoint.port,
           local_endpoint.pid, remote_id, remote_endpoint.hostname,
           remote_endpoint.port, remote_endpoint.pid, local_id, remote_id,
           socket);

  return handle;
}

const char *ToString(CloseReason reason) {
  switch(reason) {
  case CR_UserRequest:
    return "CR_UserRequest";
  case CR_CloseAll:
    return "CR_CloseAll";
  case CR_ReadError:
    return "CR_ReadError";
  case CR_WriteError:
    return "CR_WriteError";
  case CR_EndOfFile:
    return "CR_EndOfFile";
  case CR_HandshakeReadError:
    return "CR_HandshakeReadError";
  case CR_HandshakeParseError:
    return "CR_HandshakeParseError";
  case CR_HandshakeInfoMismatch:
    return "CR_HandshakeInfoMismatch";
  case CR_HandshakeRefused:
    return "CR_HandshakeRefused";
  case CR_HandshakeSynSendError:
    return "CR_HandshakeSynSendError";
  case CR_HandshakeAckSendError:
    return "CR_HandshakeAckSendError";
  }
  return "CR_Unknown";
}

void ConnectionManager::Close(ConnectionHandle handle, CloseReason reason) {
  Base::MutexAutoLock lock(m_mutex);

  LOG_INFO("closing conn(%p). Reason: %s", handle, ToString(reason));

  u16 index = FindIndex(handle);
  if(index == InvalidIndex) {
    LINK_WARN("problem closing connection %p - not found.", handle);
    return;
  }

  m_data[index].flags |= Connection::kClosed;
  LOG_INFO("marked conn(%p) for closing.");
  TryClose(index);
}

void ConnectionManager::TryClose(u16 index) {
  Connection &common = m_data[index];
  if(common.vfptr->CanClose(*this, common.detail_index)) {
    common.vfptr->DestroyConnection(*this, index);
  }
}

void ConnectionManager::Close(ConnectionHandle handle) {
  Close(handle, CR_UserRequest);
}

void ConnectionManager::CloseAll() {
  Base::MutexAutoLock lock(m_mutex);
  LOG_INFO("closing all connections");

  ConnectionHandle all_connections[m_max_connections];
  u16 num_connections = m_connection_count;
  memcpy(all_connections, m_connection_list,
         sizeof(ConnectionHandle) * num_connections);

  for(u16 i = 0; i < num_connections; ++i) {
    Close(all_connections[i], CR_CloseAll);
  }

  LOG_INFO("all connections closed");
}

std::string OperationsToString(int operations) {
  std::string result;

  if(operations & Base::Epoll::OP_READ) {
    result += "R";
  }
  if(operations & Base::Epoll::OP_WRITE) {
    result += "W";
  }
  if(operations & Base::Epoll::OP_ERROR) {
    result += "E";
  }
  return result;
}

void ConnectionManager::AcceptConnections(ConnectionListener *listener) {
  Base::Socket::Handle new_connection;
  Base::Url connectee;

  while(Base::Socket::Tcp::Accept(m_tcp.listen_socket, &new_connection,
                                  &connectee)) {

    ConnectionHandle handle = GenerateHandle();

    LOG_INFO(
        "(connect) accepted new connection: %d - %d.%d.%d.%d:%d. handle: %p",
        new_connection, connectee.GetAddress().GetA(),
        connectee.GetAddress().GetB(), connectee.GetAddress().GetC(),
        connectee.GetAddress().GetD(), connectee.GetPort(), handle);

    u16 index = AddConnection(handle);
    if(index == InvalidIndex) {
      LINK_WARN("(connect) failed to add a remote connection - add failed");
      Base::Socket::Close(new_connection);
      return;
    }

    u16 detail_index = AddRemoteConnection(m_tcp);
    Connection &common = m_data[index];

    PluginHandle remote_id = 0; // this will be updated during handshake
    common.Init(handle, m_owner, remote_id, listener, detail_index,
                0 /*kRemote | kIncoming*/);

    RemoteConnection &detail = m_tcp.connection[detail_index];
    detail = {};
    detail.socket = new_connection;
    detail.in_messages = new MessageInStream(kTcpConnectionBytes);
    detail.out_messages = new MessageOutStream(kTcpConnectionBytes);

    PluginInfo local_endpoint;
    bool has_local_info = m_plugin_dir.GetInfo(m_owner, local_endpoint);
    BASE_ASSERT(has_local_info, "failed to retrieve local plugin info");

    // create io watch for socket.
    LOG_INFO("(epoll) adding watch for sock(%d). data %d.", new_connection,
             index);
    bool epoll_added = Base::EpollAdd(
        m_tcp.epoll, new_connection, reinterpret_cast<void *>(index),
        Base::Epoll::OP_READ | Base::Epoll::OP_WRITE | Base::Epoll::OP_ERROR);
    BASE_ASSERT(epoll_added, "failed to add epoll operation for sock(%d)",
                new_connection);

    LOG_INFO("(connect) remote connection added. sock(%d). plugin: %p. id: %p "
             "(%p -> %p)",
             new_connection, m_owner, common.handle, m_owner, common.endpoint);
    LOG_INFO("(connect)	opened connection %p %s:%d(%d) -> %p %s:%d(%d) "
             "(%p -> %p). on sock(%d)",
             m_owner, local_endpoint.hostname, local_endpoint.port,
             local_endpoint.pid, remote_id, "?" /*remote_endpoint.hostname*/,
             0 /*remote_endpoint.port*/, 0 /*remote_endpoint.pid*/, handle,
             0 /*remote_handle*/, new_connection);

    // this socket will be OP_WRITE notified and have its state changed to
    // kEstablished.
    common.SetState(ConnectionState::kConnecting);
  }
}

void ConnectionManager::HandleIO(ConnectionListener *listener,
                                 time_ms timeout) {
  const s32 max_events = 32;
  FlushOutgoing();

  struct {
    ConnectionHandle handle;
    CloseReason reason;
  } dead_connections[max_events];
  s32 num_dead_connections = 0;

  auto handler = [&](void *user_data, int operations, int debug_data) {
    LOG_INFO(
        "(epoll) event on %s conn(%p) sock(%d). udata: %d, op: %s, debug data "
        "%d.",
        user_data == kListenSocketId ? "listen" : "connection",
        user_data == kListenSocketId ? 0 : m_data[(s64)user_data].handle,
        user_data == kListenSocketId
            ? m_tcp.listen_socket
            : m_tcp.connection[m_data[(s64)user_data].detail_index].socket,
        user_data, OperationsToString(operations).c_str(), debug_data);

    if(user_data == kListenSocketId) {
      BASE_ASSERT(debug_data == m_tcp.listen_socket || debug_data == 0,
                  "debug data is socket handle on osx, zero on linux %d, %d",
                  debug_data, m_tcp.listen_socket);
      // listen socket handling.
      if(operations & Base::Epoll::OP_READ) {
        // there is connection to accept.
        // data contains the size of the listen backlog
        AcceptConnections(listener);
      }
    } else if(user_data == kInterruptId) {
      // todo(kstasik): support this.
    } else {
      // non-listen socket handling.
      size_t connection_index = (size_t)user_data;
      Connection &common = m_data[connection_index];
      RemoteConnection &remote = m_tcp.connection[common.detail_index];
      Base::Socket::Handle socket = remote.socket;

      BASE_ASSERT(debug_data == 0 || socket == debug_data,
                  "epoll debug_data is zero, kqueue dabug_data is socket "
                  "handle %d, sock(%d)",
                  debug_data, socket);
      BASE_ASSERT(!(common.flags & Connection::kLocal),
                  "tcp only for remote connections. at index %zu. flags: %d",
                  connection_index, common.flags);

      LOG_INFO("socket at index %d detail %d is sock(%d)", connection_index,
               common.detail_index, socket);

      if(operations & Base::Epoll::OP_READ) {
        // data contains the number of bytes of protocol data available to read
        LOG_INFO("read");
        MessageInStream::ProcessResult socket_result =
            remote.in_messages->Process(remote.socket);
        if(socket_result == MessageInStream::RS_WOULDBLOCK) {
          if(common.state == ConnectionState::kConnecting) {
            // StartHandshake(common.handle);
            // do not set kHandshake state. OP_WRITE event will.
          }
        } /*else if(socket_result == MessageInStream::RS_EOF) {
					LOG_INFO("read to the EOF of conn(%p) sock(%d)", common.handle, remote.socket);
					dead_connections[num_dead_connections++] = {common.handle, CR_EndOfFile };
				} */ else {
          LOG_ERROR("problem processing connection %p sock(%d). closing.",
                    common.handle, remote.socket);
          dead_connections[num_dead_connections++] = {common.handle,
                                                      CR_ReadError};
          // continue; // ?
        }
        LOG_INFO("post read");
      }

      if(operations & Base::Epoll::OP_WRITE) {
        LOG_INFO("write");
        // BASE_ASSERT(common.state == ConnectionState::kConnecting);
        if(common.state == ConnectionState::kConnecting) {
          common.flags |= Connection::kWritable;
          // connection has been established. remove OP_WRITE IO watch on
          // socket.
          bool epoll_updated =
              Base::EpollUpdate(m_tcp.epoll, remote.socket, user_data,
                                Base::Epoll::OP_READ | Base::Epoll::OP_ERROR);
          BASE_ASSERT(epoll_updated, "failed to update epoll for %d.",
                      remote.socket);

          common.SetState(ConnectionState::kHandshake);
        }
      }

      /*			if(operations & Base::Epoll::OP_EOF) {
                                      // todo: if EOF is notified we need to
         read all remaining data first.

                                      LOG_INFO("sock(%d) closed with EOF.
         scheduling close for connection %p", remote.socket, common.handle);
                                      dead_connections[num_dead_connections++] =
         {common.handle, CR_EndOfFile};
                              }*/
    }
  };

  Base::MutexAutoLock lock(m_mutex);
  //	LOG_INFO("epoll wait %dms", timeout);
  int event_num = Base::EpollWait(m_tcp.epoll, timeout, max_events, handler);
  if(event_num > 0) {
    LOG_INFO("wait ended with %d", event_num);
  }

  // close dead connections.
  for(s32 conn_index = 0; conn_index < num_dead_connections; ++conn_index) {
    LOG_INFO("late closing %p", dead_connections[conn_index].handle);
    Close(dead_connections[conn_index].handle,
          dead_connections[conn_index].reason);
  }
  num_dead_connections = 0;

  // handshakes need to be processed right after receiving messages,
  // so handshake messages are consumed and not exposed to plugins.
  UpdateHandshake();
}

static const streamsize s_handshake_buffer_size = 128;
static char s_handshake_buffer[s_handshake_buffer_size];

void ConnectionManager::UpdateHandshake() {
  // connections are updated from last to first, so we can
  // safely close the connection if an error occurs and process all of them.
  for(s32 conn_index = m_connection_count - 1; conn_index >= 0; --conn_index) {
    Connection &connection = m_data[conn_index];
    if(!(connection.flags &
         Connection::kLocal)) { // handshake only for remote connections.
      RemoteConnection &remote = m_tcp.connection[connection.detail_index];

      bool error = false;

      while(!error && !(connection.flags & Connection::kAckReceived) &&
            remote.in_messages->NextMessageSize() > 0) {
        streamsize read;
        Result result = remote.in_messages->Read(
            s_handshake_buffer, s_handshake_buffer_size, &read);
        if(result != RS_SUCCESS) {
          LOG_ERROR("problem reading handshake ack size: %d/%d", read,
                    s_handshake_buffer_size);
          Close(connection.handle, CR_HandshakeReadError);
          error = true;
          continue;
        }

        ProtobufStream stream(s_handshake_buffer, read);
        handshake::Handshake message;
        if(!message.ParseFromIstream(&stream)) {
          LOG_ERROR("problem parsing handshake message");
          Close(connection.handle, CR_HandshakeParseError);
          error = true;
          continue;
        }

        if(message.type() == handshake::Handshake::kSYN) {
          const handshake::PluginInfo &msg_info = message.info();

          PluginInfo info;
          Base::String::strncpy(info.hostname, msg_info.hostname().c_str(),
                                kPluginHostnameMax);
          Base::String::strncpy(info.name, msg_info.name().c_str(),
                                kPluginNameMax);
          Base::String::strncpy(info.version, msg_info.version().c_str(),
                                kPluginVersionMax);
          info.port = msg_info.port();
          info.pid = msg_info.pid();

          PluginHandle handle = m_plugin_dir.GenerateHandle(info);
          if(m_plugin_dir.Register(handle, info)) {
            LOG_INFO("handshake registration succeeded of %s %s at %p",
                     info.name, info.version, handle);
          } else {
            LOG_INFO("handshake registration failed of %s %s at %p", info.name,
                     info.version, handle);
          }

          if(connection.endpoint == 0) {
            LOG_INFO("endpoint updated to %d", handle);
            connection.endpoint = handle;
          }

          if(connection.endpoint != handle) {
            LOG_ERROR("connection endpoint mismatch for conn(%p). %p != %p",
                      connection.handle, connection.endpoint, handle);
            Close(connection.handle, CR_HandshakeInfoMismatch);
            error = true;
            continue;
          }

          LOG_INFO("handshake for conn(%p) received kSYN", connection.handle);
          connection.flags |= Connection::kSynReceived;
        } else if(message.type() == handshake::Handshake::kACK) {
          LOG_INFO("handshake for conn(%p) received kACK", connection.handle);
          if(message.accepted()) {
            connection.flags |= Connection::kAckReceived;
            u32 establishing_time = Base::Time::GetTimeMs() - connection.time;
            LOG_INFO("conn(%p) established in %dms.", connection.handle,
                     establishing_time);
            connection.SetState(ConnectionState::kEstablished);
          } else {
            LOG_ERROR("conn(%p) refused", connection.handle);
            error = true;
            Close(connection.handle, CR_HandshakeRefused);
            continue;
          }
        }
      }

      if(connection.flags &
         Connection::kWritable) { // if writable, we can send SYN and ACK.
        if(!(connection.flags & Connection::kSynSent)) {
          LOG_INFO("handshake for conn(%p) sending kSYN", connection.handle);
          if(SendHandshakeSyn(remote)) {
            connection.flags |= Connection::kSynSent;
          } else {
            Close(connection.handle, CR_HandshakeSynSendError);
          }
        } else if((connection.flags & Connection::kSynReceived) &&
                  !(connection.flags & Connection::kAckSent)) {
          LOG_INFO("handshake for conn(%p) sending kACK", connection.handle);
          if(SendHandshakeAck(remote)) {
            connection.flags |= Connection::kAckSent;
          } else {
            Close(connection.handle, CR_HandshakeAckSendError);
          }
        }
      }
    }
  }
}

bool ConnectionManager::SendHandshakeSyn(struct RemoteConnection &remote) {
  PluginInfo info;
  if(!m_plugin_dir.GetInfo(m_owner, info)) {
    LOG_ERROR("problem getting plugin info for handshake syn");
    return false;
  }

  handshake::Handshake message;
  message.set_type(handshake::Handshake::kSYN);

  handshake::PluginInfo *plugin_info = message.mutable_info();
  plugin_info->set_name(info.name);
  plugin_info->set_version(info.version);
  plugin_info->set_hostname(info.hostname);
  plugin_info->set_port(info.port);
  plugin_info->set_pid(info.pid);
  std::string serialized;
  message.SerializeToString(&serialized);

  LOG_INFO("sending handshake plugin %s %s", info.name, info.version);

  if(!remote.out_messages->Write(
         serialized.data(), static_cast<unsigned int>(serialized.length()))) {
    LOG_ERROR("failed to send handshake syn. closing connection");
    return false;
  }
  return true;
}

bool ConnectionManager::SendHandshakeAck(struct RemoteConnection &remote) {
  handshake::Handshake response;
  response.set_type(handshake::Handshake::kACK);
  response.set_accepted(true); // todo: true or false.

  std::string serialized;
  response.SerializeToString(&serialized);

  bool ret = remote.out_messages->Write(
      serialized.data(), static_cast<unsigned int>(serialized.length()));
  if(!ret) {
    LOG_ERROR("failed to send handshake ack. closing connection");
    return false;
  }
  return true;
}

bool ConnectionManager::Send(ConnectionHandle handle, const void *data,
                             streamsize nbytes) {
  Base::MutexAutoLock lock(m_mutex);
  u16 index = FindIndex(handle);
  if(index == InvalidIndex) {
    LINK_WARN("failed to send %d bytes to %p - handle not found", nbytes,
              handle);
    return false;
  }

  if(m_data[index].state != ConnectionState::kEstablished) {
    LINK_WARN("failed to send %d bytes to %p - connection not established (%s)",
              nbytes, handle, ToString(m_data[index].state));
    return false;
  }

  u16 detail_index = m_data[index].detail_index;
  return m_data[index].vfptr->Write(*this, detail_index, data, nbytes);
}

unsigned int
ConnectionManager::TotalRecvAvailable(ConnectionHandle handle) const {
  Base::MutexAutoLock lock(m_mutex);
  u16 index = FindIndex(handle);
  if(index == InvalidIndex) {
    LINK_WARN("failed to get total recv - invalid handle %p.", handle);
    return 0;
  }

  u16 detail_index = m_data[index].detail_index;
  return m_data[index].vfptr->TotalMessageSize(*this, detail_index);
}

unsigned int
ConnectionManager::NextRecvAvailable(ConnectionHandle handle) const {
  Base::MutexAutoLock lock(m_mutex);
  u16 index = FindIndex(handle);
  if(index == InvalidIndex) {
    LINK_WARN("failed to get next recv - invalid handle %p. %p", handle,
              m_owner);
    return 0;
  }

  u16 detail_index = m_data[index].detail_index;
  return m_data[index].vfptr->NextRecvAvailable(*this, detail_index);
}

Result ConnectionManager::Recv(ConnectionHandle handle, void *bytes,
                               streamsize count, streamsize *received) {
  Base::MutexAutoLock lock(m_mutex);
  u16 index = FindIndex(handle);
  if(index == InvalidIndex) {
    LINK_WARN("failed to recv - invalid handle %p.", handle);
    return RS_FAIL;
  }

  u16 detail_index = m_data[index].detail_index;
  Result res =
      m_data[index].vfptr->Read(*this, detail_index, bytes, count, received);

  if(m_data[index].flags & Connection::kClosed) {
    TryClose(index);
  }
  return res;
}

bool ConnectionManager::GetNotification(Notification *notification) {
  BASE_ASSERT(notification);
  Base::MutexAutoLock lock(m_mutex);
  bool notify = false;

  if(!notification)
    return notify;

  // todo(kstasik): do more efficiently
  for(u32 i = 0; i < m_connection_count; ++i) {
    BASE_ASSERT(m_data[i].handle != 0);
    if(TotalRecvAvailable(m_data[i].handle) > 0) {
      notification->type = kRecvReady;
      notification->content.connection.handle = m_data[i].handle;
      notification->content.connection.endpoint = m_data[i].endpoint;
      notify = true;
      LOG_INFO("pushing recv notification for %p %s", m_data[i].handle,
               m_data[i].flags & Connection::kLocal ? "l" : "r");
      return notify;
    }
  }

  return notify;
}

void ConnectionManager::DestroyRemoteConnection(u16 index) {
  Connection &common = m_data[index];
  RemoteConnection &remote = m_tcp.connection[m_data[index].detail_index];

  BASE_ASSERT(!(common.flags & Connection::kLocal), "connection not remote");

  if(common.state != ConnectionState::kDisconnected) {
    common.SetState(ConnectionState::kDisconnected);
  }

  // remove from epoll.
  LOG_INFO("(epoll) removing watch for sock(%d).", remote.socket);
  Base::EpollRemove(m_tcp.epoll, remote.socket);

  LOG_INFO("(connect) closing sock(%d)", remote.socket);
  Base::Socket::Close(remote.socket);
  remote.socket = 0;
  delete remote.in_messages;
  delete remote.out_messages;

  LOG_INFO("destroyed remote conn(%p)", common.handle);

  u16 swapped = RemoveRemoteConnection(m_tcp, common.detail_index);
  if(swapped != InvalidIndex) {
    // fix refferences to moved element.
    for(int i = 0; i < m_connection_count; ++i) {
      if(!(m_data[i].flags & Connection::kLocal) &&
         m_data[i].detail_index == swapped) {
        m_data[i].detail_index = common.detail_index;
      }
    }
  }
  RemoveConnection(index);
}

void ConnectionManager::DestroyLocalConnection(u16 index) {
  Connection &common = m_data[index];
  LocalConnection &detail = m_inproc.connection[common.detail_index];

  BASE_ASSERT(common.flags & Connection::kLocal, "only local connection");

  if(common.state != ConnectionState::kDisconnected) {
    common.SetState(ConnectionState::kDisconnected);
  }

  // set other endpoint's state to kDisconnected.
  u16 dest_index = detail.remote_manager->FindIndex(detail.remote_handle);
  if(dest_index != InvalidIndex) {
    Connection &destination = detail.remote_manager->m_data[dest_index];
    destination.SetState(ConnectionState::kDisconnected);

    // unlink the other endpoint, because we're about to remove this connection.
    LocalConnection &remote_detail =
        detail.remote_manager->m_inproc.connection[destination.detail_index];
    remote_detail.remote_handle = 0;
    remote_detail.out_messages = nullptr;
    // remote_detail.remote_manager = nullptr; // do we clear that? what if we
    // want to reconnect?

    LOG_INFO("destroyed local conn(%p).", common.handle);

    delete detail.in_messages;

    u16 swapped = RemoveLocalConnection(m_inproc, common.detail_index);
    if(swapped != InvalidIndex) {
      // fix refferences to moved element.
      for(int i = 0; i < m_connection_count; ++i) {
        if(m_data[i].flags & Connection::kLocal &&
           m_data[i].detail_index == swapped) {
          m_data[i].detail_index = common.detail_index;
        }
      }
    }

    RemoveConnection(index);
  }
}

void ConnectionManager::FlushOutgoing() {
  // todo(kstasik): optimise this.
  // iterating in recerse to safely remove connection on failure.
  for(s32 i = m_connection_count - 1; i >= 0; --i) {
    if(!(m_data[i].flags & Connection::kLocal)) {
      RemoteConnection &remote = m_tcp.connection[m_data[i].detail_index];
      bool result = remote.out_messages->Flush(remote.socket);
      if(!result) {
        LOG_ERROR("problem flushing connection on sock(%d)", remote.socket);
        remote.out_messages->Clear();
        Close(m_data[i].handle, CR_WriteError);
      }
      if(m_data[i].flags & Connection::kClosed) {
        TryClose(i); // todo(kstasik): type known here. can try closing remote.
      }
    }
  }
}

u16 ConnectionManager::FindIndex(ConnectionHandle handle) const {
  for(u16 i = 0; i < m_connection_count; ++i) {
    if(handle == m_connection_list[i]) {
      return i;
    }
  }
  return InvalidIndex;
}

u16 ConnectionManager::AddConnection(ConnectionHandle handle) {
  if(FindIndex(handle) != InvalidIndex) {
    return InvalidIndex; // given handle is already registered.
  }
  if(m_connection_count == m_max_connections) {
    return InvalidIndex; // max number of connections reached.
  }
  // add handle, return index.
  u16 index = m_connection_count;
  m_connection_list[index] = handle;
  m_connection_count++;
  LOG_INFO("(connect) %p added connection %p at %d", m_owner, handle, index);
  return index;
}

void ConnectionManager::RemoveConnection(u16 remove_index) {
  BASE_ASSERT(remove_index < m_connection_count, "remove index out of bounds");
  u16 last_index = m_connection_count - 1;

  ConnectionHandle handle = m_connection_list[remove_index];

  if(remove_index != last_index) {
    // move last element into freed slot.
    m_connection_list[remove_index] = m_connection_list[last_index];

    LOG_INFO("(connect) %p moved connection %p to %d", m_owner,
             m_connection_list[remove_index], remove_index);

    // move data to new position, because index refference changed.
    m_data[remove_index] = m_data[last_index];

    // fix epoll user data if the connection is remote.
    if(!(m_data[remove_index].flags & Connection::kLocal)) {
      RemoteConnection &remote =
          m_tcp.connection[m_data[remove_index].detail_index];

      LOG_INFO("(epoll) updating watch data for sock(%d) from %d to %d.",
               remote.socket, last_index, remove_index);
      bool epoll_updated = Base::EpollUpdate(
          m_tcp.epoll, remote.socket, reinterpret_cast<void *>(remove_index),
          Base::Epoll::OP_READ | Base::Epoll::OP_ERROR);
      BASE_ASSERT(epoll_updated,
                  "failed to update epoll for sock(%d) from %d to %d",
                  remote.socket, last_index, remove_index);
    }
  }
  LOG_INFO("(connect) %p removed connection %p at %d. flags: %d", m_owner,
           handle, remove_index);
  --m_connection_count;
}

} // namespace Link
