/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "rest_plugin.h"

#include "base/core/str.h"
#include "base/math/crc.h"
#include "link/plugin_log.h"

#include "common/json/json_reader.h"
#include "common/json/json_writer.h"
#include "json_utils.h"

#include <stdio.h>
#include <stdlib.h>

namespace Link {

RestPlugin::RestPlugin()
    : SimplePlugin(kUpdateDeltaMs), m_command_list(m_server) {
  m_recv_buffer = malloc(kRecvBufferSize);
}

RestPlugin::~RestPlugin() { free(m_recv_buffer); }

bool RestPlugin::OnStartup(const char *config, streamsize nbytes) {
  if(!RegisterCommand(&m_command_list)) {
    return false;
  }

  const size_t kMaxArgLength = 64;
  char num_threads[kMaxArgLength];
  char listen_ports[kMaxArgLength];
  const char *kServerOptions[] = {"num_threads",
                                  num_threads,
                                  "listening_ports",
                                  listen_ports,
                                  "access_control_allow_origin",
                                  "*",
                                  NULL};

  Base::String::ToString(20, num_threads, kMaxArgLength);
  Base::String::ToString(8080, listen_ports, kMaxArgLength);

  bool started = m_server.Start(kServerOptions);
  if(!started) {
    // port 8080 might have been taken. let's try ephemeral port.
    Base::String::ToString(0, listen_ports, kMaxArgLength);
    started = m_server.Start(kServerOptions);

    if(!started) {
      PLUGIN_ERROR("failed to start http server");
    }
  }

  // started = true;
  return started;
}

void RestPlugin::OnShutdown() {

  UnregisterCommand(&m_command_list);
  m_server.Stop();
}

void RestPlugin::OnRecvReady(const ConnectionNotification &notif) {
  SimplePlugin::Recv(notif.handle, m_recv_buffer, kRecvBufferSize,
                     [&](void *buffer, unsigned int nbytes) {
                       ParseDataReceived(buffer, nbytes, notif.handle,
                                         notif.endpoint);
                     });
}

void RestPlugin::OnNotification(const Notification &notif) {
  RestClient::ProcessNotification(notif);

  // if any plugin disconnects from rest plugin, we need to remove all related
  // rest commands.
  if(notif.type == kDisconnected) {
    m_server.UnregisterAll(notif.content.connection.handle);
  }
}

bool CommandListCmd::OnCommand(const std::string &query_string,
                               const std::string &post_data,
                               std::string *response_data) {
  (void)query_string;
  (void)post_data;

  LinkConfiguration config = GetConfiguration();

  JsonWriter writer(*response_data);
  writer.Write("plugin", config.info);
  m_server.WriteAvailableCommands(&writer);
  return true;
}

void RestPlugin::ParseDataReceived(void *buffer, unsigned int nbytes,
                                   ConnectionHandle connection,
                                   PluginHandle plugin) {
  JsonReader reader;

  if(reader.Parse((const char *)buffer, nbytes)) {
    std::string type;
    if(!reader.Read<std::string>("type", &type)) {
      return;
    }

    if(type == "register_command") {
      // command registration message arrived.
      // register command with a given connection_handle
      // and send "register_command_response back to requester.
      std::string command_name = reader.Read<std::string>("command_name", "");
      PLUGIN_INFO("received register request for %s. from plugin: %p.",
                  command_name.c_str(), plugin);
      OnRegisterCommand(command_name.c_str(), connection, plugin);
    } else if(type == "response_data") {
      // a response to http request arrived from a plugin.
      // find an appropriate http connection if it's still opened
      // and send the response back.
      int request_id = reader.Read<int>("request_id", (int)-1);
      std::string response_data = reader.Read<std::string>("data", "");
      PLUGIN_INFO("received response data for request %d. from plugin %p.",
                  request_id, plugin);
      OnResponseData(request_id, response_data.c_str(),
                     (int)response_data.length(), connection);
    } else if(type == "unregister_command") {
      int command_id;
      if(reader.Read("command_id", &command_id)) {
        PLUGIN_INFO(
            "receive unregister request for command id %d. from plugin %p.",
            command_id, plugin);
        OnUnregisterCommand(command_id, connection);
      }
    }
  } else {
    PLUGIN_ERROR("Cannot parse received data: \"%s\"", buffer);
  }
}

void RestPlugin::OnRegisterCommand(const char *command_name,
                                   ConnectionHandle connection,
                                   PluginHandle plugin) {
  std::string output;
  JsonWriter writer(output);

  writer.Write<const char *>("type", "register_command_response");

  bool ok = false;
  PluginInfo info;
  ok = 0 == GetPluginInfo(plugin, &info);

  PLUGIN_INFO("registering command %s for plugin %s(%s).", command_name,
              info.name, info.version);

  writer.Write<const char *>("command_name", command_name);

  std::string command_long =
      ok ? GetFullCommandName(command_name, info) : command_name;
  // registering a command.
  ok &= m_server.Register(command_long.c_str(), connection);

  if(ok) {
    writer.Write<int>("command_id", Base::Math::crc(command_long.c_str()));
  } else {
    writer.Write<int>("command_id", 0);
    writer.Write<const char *>("error", "error occurred");
  }

  writer.Finalize();

  PLUGIN_INFO("sending %s registration response for %s",
              ok ? "success" : "failure",
              ok ? command_long.c_str() : command_name);

  if(0 != Send(connection, output.c_str(), (unsigned int)output.length())) {
    PLUGIN_ERROR("send failed to send response to a plugin.");
    CloseConnection(connection);
  }
}

void RestPlugin::OnUnregisterCommand(int command_id,
                                     ConnectionHandle connection) {
  (void)connection;
  m_server.Unregister(command_id);
}

void RestPlugin::OnResponseData(int request_id, const char *response_data,
                                int response_length,
                                ConnectionHandle connection) {
  PLUGIN_INFO("sending http response id: %d. \"%s\"", request_id,
              response_data);
  m_server.SendResponse(connection, request_id, response_data, response_length);
}

std::string RestPlugin::GetFullCommandName(const char *cmd_name,
                                           const PluginInfo &info) {
  std::string result = "/";

  bool correct_plugin =
      Base::String::Compare(RestPlugin::Name, info.name) == 0 ||
      Base::String::Compare(RestPlugin::Version, info.version) == 0;

  if(!correct_plugin) {
    result += cmd_name;
  } else {
    char pid[256];
    snprintf(pid, 256, "%i", info.pid);
    // todo: result += info.host; result += "/";

    // simplified command-list. hack for now..
    if(0 != Base::String::Compare(cmd_name, "command-list")) {
      result += info.hostname;
      result += "/";
      result += pid;
      result += "/";
      result += info.name;
      result += "/";
      result += info.version;
      result += "/";
    }

    result += cmd_name;
  }
  return result;
}

} // namespace Link
