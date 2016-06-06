/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gate_plugin.h"

#include "link/plugin_log.h"
#include "base/core/str.h"
#include "base/math/crc.h"

#include "tinyxml2.h"

#include <stdlib.h>
#include <stdio.h>

#include "common/protobuf_stream.h"
#include "common/json/json_writer.h"
#include "protocol/gate.pb.h"
#include "gate_users.h"

namespace Link {
namespace Gate {

void OnConnected(TCPServer::Handle handle, const Base::Socket::Address &address,
                 void *context) {
  PLUGIN_INFO("connection established %d. ip %s", handle,
              Base::Socket::Print(address));
  GatePlugin *this_ = static_cast<GatePlugin *>(context);
  this_->AddConnection(handle, address);
}

void OnDisconnected(TCPServer::Handle handle, TCPServer::CloseReason reason,
                    const Base::Socket::Address &address, void *context) {
  PLUGIN_INFO("disconnected %d. reason %s. ip %s", handle,
              TCPServer::ToString(reason), Base::Socket::Print(address));
  GatePlugin *this_ = static_cast<GatePlugin *>(context);
  this_->RemConnection(handle);
}
void OnMessage(TCPServer::Handle handle, void *buffer, u32 nbytes,
               void *context) {
  PLUGIN_INFO("data on connection %d. bytes %d", handle, nbytes);
  GatePlugin *this_ = static_cast<GatePlugin *>(context);
  this_->HandleMessage(handle, buffer, nbytes);
}

GatePlugin::GatePlugin()
    : SimplePlugin(kUpdateDeltaMs), m_list_cmd(*this), m_disconnect_cmd(*this),
      m_logout_cmd(*this) {
  m_recv_buffer = malloc(kRecvBufferSize);
}

GatePlugin::~GatePlugin() { free(m_recv_buffer); }

bool GatePlugin::OnStartup(const char *config, streamsize nbytes) {

  if(!RegisterCommand(&m_list_cmd) || !RegisterCommand(&m_disconnect_cmd) ||
     !RegisterCommand(&m_logout_cmd)) {
    return false;
  }
  if(!config || nbytes == 0) {
    PLUGIN_ERROR("no config");
    return false;
  }

  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError err = doc.Parse(config, nbytes);

  if(err != tinyxml2::XML_SUCCESS) {
    PLUGIN_ERROR("problem parsing config: %s(%d)", doc.ErrorName(), err);
    return false;
  }

  u16 port = 0;

  tinyxml2::XMLElement *root = doc.RootElement();
  if(root->Attribute("port")) {
    port = root->IntAttribute("port");
    PLUGIN_INFO("port read %d", port);
  } else {
    PLUGIN_WARN("no port specified, defaulting to 0");
  }

  if(!root->Attribute("max_connections")) {
    PLUGIN_ERROR("maximum connection count not specified");
    return false;
  }
  u32 max_connections = root->IntAttribute("max_connections");
  PLUGIN_INFO("maximum number of connections: %d", max_connections);

  if(max_connections == 0) {
    PLUGIN_ERROR("invalid number of max connections: %d", max_connections);
    return false;
  }

  m_users = new Users(max_connections);
  if(m_users == nullptr) {
    PLUGIN_ERROR("problem creating users data");
    return false;
  }

  TCPServer::Callbacks callbacks = {Link::Gate::OnConnected,
                                    Link::Gate::OnDisconnected,
                                    Link::Gate::OnMessage};

  m_conn = new TCPServer(max_connections, callbacks, this);

  if(!m_conn->CreateListenSocket(port)) {
    delete m_conn;
    PLUGIN_ERROR("failed to create listen port");
    return false;
  }

  PLUGIN_INFO("gate listening on port: %d", port);

  return true;
}

void GatePlugin::OnShutdown() {
  UnregisterCommand(&m_list_cmd);
  UnregisterCommand(&m_disconnect_cmd);
  UnregisterCommand(&m_logout_cmd);

  m_conn->CloseAll(TCPServer::CloseReason::kShutdown);
  delete m_users;
  delete m_conn;
}

void GatePlugin::OnUpdate(unsigned int dt) {
  (void)dt;
  m_conn->HandleIO(0);
}

void GatePlugin::OnRecvReady(const ConnectionNotification &notif) {
  SimplePlugin::Recv(notif.handle, m_recv_buffer, kRecvBufferSize,
                     [&](void *buffer, unsigned int nbytes) {
                       ParseDataReceived(buffer, nbytes, notif.handle,
                                         notif.endpoint);
                     });
}

void GatePlugin::OnNotification(const Notification &notif) {
  ProcessNotification(notif);
}

void GatePlugin::OnPluginConnected(const ConnectionNotification &notif) {
  if(GetRestConnection() == notif.handle) {
    return; // ignore rest plugin connection.
  }
}

void GatePlugin::OnConnected(const ConnectionNotification &notif) {
  if(GetRestConnection() == notif.handle) {
    return; // ignore rest plugin connection.
  }
}

void GatePlugin::OnDisconnected(const ConnectionNotification &notif) {
  if(GetRestConnection() == notif.handle) {
    return; // ignore rest plugin connection.
  }
}

void GatePlugin::ParseDataReceived(void *buffer, unsigned int nbytes,
                                   ConnectionHandle connection,
                                   PluginHandle plugin) {}

void GatePlugin::AddConnection(TCPServer::Handle handle,
                               const Base::Socket::Address &address) {
  m_users->AddConnection(handle, address);
}

void GatePlugin::RemConnection(TCPServer::Handle handle) {
  m_users->RemConnection(handle);
}

void GatePlugin::HandleMessage(TCPServer::Handle handle, void *data,
                               u32 nbytes) {
  // Connection& conn = m_users->GetConnection(handle);

  ProtobufStream stream(data, nbytes);
  gate::ClientMessage msg;
  if(!msg.ParseFromIstream(&stream)) {
    PLUGIN_ERROR("protocol error");
    m_conn->Close(handle, TCPServer::CloseReason::kProtocolError);
    return;
  }
  gate::ClientMessage::Type type = msg.type();
  const char *protoc = msg.protocol_version().c_str();
  PLUGIN_INFO("message type: %d, protocol version: %s", type, protoc);

  switch(type) {
  case gate::ClientMessage::kLogin: {
    const gate::Login &login = msg.login();
    gate::ServerMessage response;
    response.set_type(gate::ServerMessage::kLoggedIn);
    response.set_protocol_version(kProtocolVersion);
    gate::LoggedIn &logged_in = *response.mutable_logged_in();
    logged_in.set_index(login.index());

    index_t index = m_users->AddUser(handle, login.username().c_str());
    logged_in.set_user_id(index);
    // todo(kstasik): validate index. send it back.
    if(index == kInvalidUserHandle) {
      PLUGIN_ERROR("problem logging in '%s'", login.username().c_str());
    }
    PLUGIN_INFO("user '%s' logged in", login.username().c_str());
    std::string serialized;
    response.SerializeToString(&serialized);
    if(!m_conn->Send(handle, static_cast<const void *>(serialized.c_str()),
                     serialized.length())) {
      m_conn->Close(handle,
                    TCPServer::CloseReason::kProtocolError); // todo(kstasik):
                                                             // new error type
    }
  } break;
  case gate::ClientMessage::kLogout: {
    const gate::Logout &logout = msg.logout();
    gate::ServerMessage response;
    response.set_type(gate::ServerMessage::kLoggedOut);
    response.set_protocol_version(
        kProtocolVersion); // todo(kstasik): refactor protoc version
    gate::LoggedOut &logged_out = *response.mutable_logged_out();
    logged_out.set_user_id(logout.user_id());
    if(!m_users->RemUser(handle, logout.user_id())) {
      PLUGIN_ERROR("problem logging out handle: %d index %d", handle,
                   logout.user_id());
      // todo(kstasik): handle error
    } else {
      PLUGIN_INFO("handle %d index %d logged out", handle, logout.user_id());
    }
    std::string serialized;
    response.SerializeToString(&serialized);
    if(!m_conn->Send(handle, static_cast<const void *>(serialized.c_str()),
                     serialized.length())) {
      m_conn->Close(handle,
                    TCPServer::CloseReason::kProtocolError); // todo(kstasik):
                                                             // new error type
    }
  } break;
  default:
    break;
  }
}

bool GatePlugin::Disconnect(TCPServer::Handle id) {
  if(!m_users->IsConnected(id)) {
    PLUGIN_ERROR("not connected %d.", id);
    return false;
  }
  PLUGIN_INFO("closing connection id: %d", id);
  m_conn->Close(id, TCPServer::CloseReason::kRESTfulAPI);
  return true;
}

bool GatePlugin::Logout(UserId id) {
  if(!m_users->IsLoggedIn(id)) {
    PLUGIN_ERROR("user not logged in %d", id);
    return false;
  }

  Connection &conn = m_users->GetConnectionByUserId(id);
  bool res = m_users->RemUser(conn.GetHandle(), id);
  if(!res) {
    PLUGIN_ERROR("problem removing user");
    return false;
  }

  gate::ServerMessage response;
  response.set_type(gate::ServerMessage::kLoggedOut);
  response.set_protocol_version(kProtocolVersion);
  gate::LoggedOut &logged_out = *response.mutable_logged_out();
  logged_out.set_user_id(id);
  std::string serialized;
  response.SerializeToString(&serialized);
  if(!m_conn->Send(conn.GetHandle(),
                   static_cast<const void *>(serialized.c_str()),
                   serialized.length())) {
    m_conn->Close(
        conn.GetHandle(),
        TCPServer::CloseReason::kWriteFailure); // todo(kstasik): new error type
    return false;
  }
  return true;
}

void GatePlugin::WriteUserList(std::string *data) { m_users->Write(data); }

} // namespace Gate
} // namespace Link

const char *SimplePlugin::Name = "gate";
const char *SimplePlugin::Version = "0.1";

SimplePlugin *SimplePlugin::CreatePlugin() {
  return new Link::Gate::GatePlugin();
}

void SimplePlugin::DestroyPlugin(SimplePlugin *plugin) { delete plugin; }
