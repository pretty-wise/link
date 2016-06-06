/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "common/tcp_server.h"
#include "common/message_stream.h"
#include "base/core/assert.h"
#include "base/core/log.h"

const Base::LogChannel kTcpServer("tcp_server");

#if 1
#define LOG_EPOLL(...) BASE_DEBUG(kTcpServer, __VA_ARGS__)
#define LOG_CONNECTION(...) BASE_INFO(kTcpServer, __VA_ARGS__)
#else
#define LOG_EPOLL(...)
#define LOG_CONNECTION(...)
#endif
namespace Link {

static void *kListenSocketId = (void *)-1;
static const u32 kEpollMaxEvents = 32;
static const u32 kTcpConnectionBytes = 10 * 1024;

struct Connection {
  Base::Socket::Handle socket;
  Base::Socket::Address address;
  Link::MessageInStream *in_messages;
  Link::MessageOutStream *out_messages;
};

TCPServer::TCPServer(u32 max_connections, Callbacks cbs, void *context)
    : m_capacity(max_connections), m_next(0), m_cbs(cbs), m_context(context) {
  BASE_ASSERT(cbs.on_connected != nullptr);
  BASE_ASSERT(cbs.on_disconnected != nullptr);
  BASE_ASSERT(cbs.on_message != nullptr);

  m_data = new Connection[max_connections];

  m_listen_socket = Base::Socket::Tcp::Open();
  m_epoll = Base::EpollCreate(kEpollMaxEvents);

  for(u32 i = 0; i < max_connections; ++i) {
    m_data[i].in_messages = new Link::MessageInStream(kTcpConnectionBytes);
    m_data[i].out_messages = new Link::MessageOutStream(kTcpConnectionBytes);
    m_data[i].socket = Base::Socket::InvalidHandle;
  }
  m_buffer = new s8[kTcpConnectionBytes];
}

TCPServer::~TCPServer() {
  Base::Socket::Close(m_listen_socket);
  Base::EpollDestroy(m_epoll);

  for(u32 i = 0; i < m_capacity; ++i) {
    BASE_ASSERT(m_data[i].socket == Base::Socket::InvalidHandle);
    delete m_data[i].in_messages;
    delete m_data[i].out_messages;
  }
  delete[] m_data;
  delete[] m_buffer;
}

bool TCPServer::CreateListenSocket(u16 &port) {
  if(!Base::Socket::Tcp::Listen(m_listen_socket, &port)) {
    BASE_LOG("failed to listen on socket");
    return false;
  }

  LOG_EPOLL("(epoll) adding watch for sock(%d). data %d.", m_listen_socket,
            kListenSocketId);
  if(!Base::EpollAdd(m_epoll, m_listen_socket, kListenSocketId,
                     Base::Epoll::OP_ALL)) {
    BASE_LOG("failed to add listen socket to epoll");
    return false;
  }
  return true;
}

TCPServer::Handle TCPServer::GenerateHandle() {
  u32 index = m_next;
  do {
    if(m_data[index].socket == Base::Socket::InvalidHandle) {
      m_next = index + 1;
      if(m_next == m_capacity) {
        m_next = 0; // wrap
      }
      return index;
    }

    if(++index == m_capacity) {
      index = 0; // wrap
    }
  } while(index != m_next);
  return static_cast<Handle>(-1);
}

void TCPServer::AcceptConnection() {
  Base::Socket::Handle new_connection;
  Base::Socket::Address connectee;

  if(Base::Socket::Tcp::Accept(m_listen_socket, &new_connection, &connectee)) {
    Handle index = GenerateHandle();

    if(index == static_cast<Handle>(-1)) {
      BASE_LOG("cannot accept connection. capacity of %d reached", m_capacity);
      return;
    }

    BASE_ASSERT(m_data[index].socket == Base::Socket::InvalidHandle);

    Connection &data = m_data[index];
    data.socket = new_connection;
    data.address = connectee;
    LOG_CONNECTION("accepted new connection: %d - %s", index,
                   Base::Socket::Print(connectee));

    m_cbs.on_connected(index, connectee, m_context);

    // create io watch for socket.
    LOG_EPOLL("(epoll) adding watch for sock(%d). at %d.", new_connection,
              index);
    bool epoll_added = Base::EpollAdd(
        m_epoll, new_connection, reinterpret_cast<void *>(index),
        Base::Epoll::OP_READ | Base::Epoll::OP_WRITE | Base::Epoll::OP_ERROR);
    BASE_ASSERT(epoll_added, "failed to add epoll operation for sock(%d)",
                new_connection);

    // this socket will be OP_WRITE notified and have its state changed to
    // kEstablished.
  }
}

void TCPServer::HandleIO(time_ms timeout) {
  const s32 max_events = 32;

  struct {
    Handle index;
    CloseReason reason;
  } dead_connections[max_events];
  s32 num_dead_connections = 0;

  struct {
    Handle index;
  } read_connections[max_events];
  s32 num_read_connections = 0;

  auto handler = [&](void *user_data, int operations, int debug_data) {
    char debug_op[4];
    Base::Epoll::ToString(operations, debug_op);
    LOG_EPOLL("(epoll) event on %s socket. index %d: operation: %s",
              user_data == kListenSocketId ? "listen" : "connection",
              (u64)user_data, debug_op);

    if(user_data == kListenSocketId) {
      BASE_ASSERT(debug_data == m_listen_socket || debug_data == 0,
                  "debug data is socket handle on osx, zero on linux %d, %d",
                  debug_data, m_listen_socket);
      // listen socket handling.
      if(operations & Base::Epoll::OP_READ) {
        // there is connection to accept.
        // data contains the size of the listen backlog
        AcceptConnection();
      }
    } else {
      // non-listen socket handling.
      size_t connection_index = (size_t)user_data;
      Connection &conn = m_data[connection_index];
      Base::Socket::Handle socket = conn.socket;

      BASE_ASSERT(debug_data == 0 || socket == debug_data,
                  "epoll debug_data is zero, kqueue dabug_data is socket "
                  "handle %d, sock(%d)",
                  debug_data, socket);
      if(operations & Base::Epoll::OP_READ) {
        // data contains the number of bytes of protocol data available to read

        Link::MessageInStream::ProcessResult socket_result =
            conn.in_messages->Process(conn.socket);

        if(socket_result == Link::MessageInStream::RS_WOULDBLOCK ||
           socket_result == Link::MessageInStream::RS_EOF) {
          LOG_CONNECTION("read %d", conn.in_messages->NextMessageSize());
          read_connections[num_read_connections++] = {
              static_cast<Handle>(connection_index)};
        }
        if(socket_result == Link::MessageInStream::RS_EOF) {
          LOG_CONNECTION("eof %d", conn.in_messages->NextMessageSize());
          dead_connections[num_dead_connections++] = {
              static_cast<Handle>(connection_index), CloseReason::kEnded};
        } else if(socket_result == Link::MessageInStream::RS_ERROR) {
          dead_connections[num_dead_connections++] = {
              static_cast<Handle>(connection_index), CloseReason::kReadFailure};
          BASE_LOG("problem processing connection %zu", connection_index);
        }
      }
    }
  };

  int event_num = Base::EpollWait(m_epoll, timeout, max_events, handler);
  BASE_ASSERT(event_num >= 0);
  if(event_num > 0) {
    LOG_EPOLL("wait ended with %d number of events.", event_num);
  }

  for(s32 conn_index = 0; conn_index < num_read_connections; ++conn_index) {
    Handle handle = read_connections[conn_index].index;
    Connection &conn = m_data[handle];
    while(conn.in_messages->NextMessageSize() > 0) {
      BASE_ASSERT(conn.in_messages->NextMessageSize() < kTcpConnectionBytes);
      streamsize read = 0;
      Result res = conn.in_messages->Read(m_buffer, kTcpConnectionBytes, &read);
      BASE_ASSERT(res != RS_BUFFER_TOO_SMALL);
      if(res == RS_SUCCESS) {
        m_cbs.on_message(handle, m_buffer, read, m_context);
      }
    }
  }
  // close dead connections.
  for(s32 conn_index = 0; conn_index < num_dead_connections; ++conn_index) {
    LOG_CONNECTION("late closing %p", dead_connections[conn_index].index);
    Close(dead_connections[conn_index].index,
          dead_connections[conn_index].reason);
  }
  num_dead_connections = 0;

  // todo(kstasik): loop only over those that need serving.
  for(u32 i = 0; i < m_capacity; ++i) {
    if(m_data[i].socket != Base::Socket::InvalidHandle) {
      if(!m_data[i].out_messages->Flush(m_data[i].socket)) {
        Close(static_cast<Handle>(i), CloseReason::kWriteFailure);
      }
    }
  }
}

void TCPServer::CloseAll(CloseReason reason) {
  for(u32 i = 0; i < m_capacity; ++i) {
    if(m_data[i].socket != Base::Socket::InvalidHandle) {
      Close(static_cast<Handle>(i), reason);
    }
  }
}

void TCPServer::Close(Handle handle, CloseReason reason) {
  if(m_data[handle].socket == Base::Socket::InvalidHandle) {
    return; // already closed
  }

  Base::EpollRemove(m_epoll, m_data[handle].socket);
  Base::Socket::Close(m_data[handle].socket);

  m_cbs.on_disconnected(handle, reason, m_data[handle].address, m_context);

  m_data[handle].socket = Base::Socket::InvalidHandle;
  LOG_CONNECTION("closed connection %d. reason: %s", handle, ToString(reason));
}

bool TCPServer::Send(Handle handle, const void *data, streamsize nbytes) {
  if(m_data[handle].socket == Base::Socket::InvalidHandle) {
    return false; // handle already closed.
  }
  if(!m_data[handle].out_messages->Write(data, nbytes)) {
    return false; // problem writing message.
  }
  return true;
}

const char *TCPServer::ToString(CloseReason r) {
  static const char *text[] = {"kShutdown",      "kEnded",
                               "kReadFailure",   "kWriteFailure",
                               "kProtocolError", "kRESTfulAPI"};
  return text[static_cast<u32>(r)];
}

} // namespace Link
