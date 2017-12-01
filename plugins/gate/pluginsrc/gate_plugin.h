/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "plugin_interface.h"
#include "base/network/url.h"
#include "common/tcp_server.h"
#include "common/json/json_writer.h"
#include "plugin/gate/gate_defs.h"
#include "plugin/rest/rest_client.h"
#include "gate_commands.h"
#include "gate_users.h"

struct redisContext;

namespace Link {
namespace Gate {
/*
        GatePlugin accepts incoming TCP connections and allows allows
   kMaxNumUsers to
        authenticate per connection.
        A TCP connection is described with: TCPServer::Handle, Base::Url.
        A user is described with:
*/

class GatePlugin : public SimplePlugin, public RestClient {
public:
  GatePlugin();
  ~GatePlugin();
  bool OnStartup(const char *config, streamsize nbytes) override;
  void OnShutdown() override;

  void OnNotification(const Notification &notif) override;
  void OnRecvReady(const ConnectionNotification &notif) override;
  void OnUpdate(unsigned int dt) override;
  void OnPluginConnected(const ConnectionNotification &notif) override;
  void OnConnected(const ConnectionNotification &notif) override;
  void OnDisconnected(const ConnectionNotification &notif) override;

  enum CONSTANTS { kUpdateDeltaMs = 10, kRecvBufferSize = 1024 };

  void AddConnection(TCPServer::Handle handle,
                     const Base::Socket::Address &address);
  void RemConnection(TCPServer::Handle handle);
  void HandleMessage(TCPServer::Handle handle, void *data, u32 nbytes);

  void WriteUserList(std::string *data);
  bool Disconnect(TCPServer::Handle id);
  bool Logout(UserId id);

private:
  void ParseDataReceived(void *buffer, unsigned int nbytes,
                         ConnectionHandle connection, PluginHandle plugin);

  bool RedisConnectBlocking(const char *hostname, u16 port);
  void RedisDisconnect();

private:
  TCPServer *m_conn;
  void *m_recv_buffer;
  redisContext *m_redis;

  Users *m_users;

  ListUsersCmd m_list_cmd;
  DisconnectCmd m_disconnect_cmd;
  LogoutCmd m_logout_cmd;
};

} // namespace Gate
} // namespace Link
