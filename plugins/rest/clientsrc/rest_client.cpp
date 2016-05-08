/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "plugin/rest/rest_client.h"
#include "plugin/rest/rest_command.h"
#include "common/json/json_writer.h"
#include "common/json/json_reader.h"
#include "link/plugin_log.h"
#include "base/core/assert.h"

#include "plugin_interface.h"

namespace Link {

RestClient::RestClient()
    : m_rest_watch(0), m_rest_connection(0), m_registered(nullptr) {
  m_rest_watch = CreateWatch("rest", "*", "*");
  BASE_ASSERT(m_rest_watch != 0);

  m_recv_buffer = malloc(kRecvBufferSize);
}

RestClient::~RestClient() {
  if(m_rest_watch) {
    CloseWatch(m_rest_watch);
    m_rest_watch = 0;
  }

  if(m_rest_connection) {
    CloseConnection(m_rest_connection);
    m_rest_connection = 0;
  }

  free(m_recv_buffer);
}

bool RestClient::RegisterCommand(RestCommand *command) {
  if(!command) {
    PLUGIN_WARN("rest command not specified");
    return false;
  }

  command->m_next = m_registered;
  command->m_prev = nullptr;
  if(m_registered)
    m_registered->m_prev = command;
  m_registered = command;

  PLUGIN_INFO("rest command registration requested for: %s", command->Name());

  SendRegisterRequest(command);
  return true;
}

void RestClient::UnregisterCommand(RestCommand *command) {
  if(!command) {
    PLUGIN_WARN("rest command not specified");
    return;
  }

  PLUGIN_INFO("rest command unregistration requested for: %s", command->Name());

  if(command->m_prev) {
    command->m_prev->m_next = command->m_next;
  }
  if(command->m_next) {
    command->m_next->m_prev = command->m_prev;
  }
  if(m_registered == command) {
    m_registered = command->m_next;
  }
  command->m_next = command->m_prev = 0;
  m_commands.erase(command->m_id);
  SendUnregisterRequest(command);
}

void RestClient::ProcessNotification(const Notification &notification) {
  switch(notification.type) {
  case kWatch:
    if(notification.content.watch.handle == m_rest_watch &&
       notification.content.watch.plugin_state == kPluginAvailable) {
      // rest plugin available, connect!
      m_rest_connection = Connect(notification.content.watch.plugin);
      if(m_rest_connection) {
        PLUGIN_INFO("rest plugin available, connecting...");
      }
    }
    break;
  case kEstablished:
    if(notification.content.connection.handle == m_rest_connection) {
      PLUGIN_INFO("connected to rest plugin.");
      // push our command registry to the rest plugin
      RetryRegistrations();
    }
    break;
  case kDisconnected:
    // properly handle remote closure of our connection handle
    if(m_rest_connection == notification.content.connection.handle) {
      CloseConnection(notification.content.connection.handle);
      m_rest_connection = 0;
    }
    break;
  case kRecvReady:
    ProcessRecv(notification.content.connection);
    break;
  default:
    // ignore the notification... it doesn't matter to us
    break;
  }
}

void RestClient::SendRegisterRequest(RestCommand *command) {
  if(0 == m_rest_connection) {
    // can't do anything if we don't have a connection
    return;
  }
  std::string request;
  request.reserve(1024);

  JsonWriter writer(request);
  writer.Write("type", "register_command");
  writer.Write("command_name", command->Name());
  writer.Finalize();

  PLUGIN_INFO("sending rest registration for %s. id %d. to %d.",
              command->Name(), m_rest_connection);
  Send(m_rest_connection, request.c_str(),
       static_cast<unsigned int>(request.size()));
}

void RestClient::RetryRegistrations() {
  RestCommand *command = m_registered;
  while(command) {
    // if the command hasn't been registered yet, retry!
    if(m_commands.end() == m_commands.find(command->m_id)) {
      SendRegisterRequest(command);
    }
    command = command->m_next;
  }
}

void RestClient::SendUnregisterRequest(RestCommand *command) {
  if(0 == m_rest_connection) {
    // can't do anything if we don't have a connection
    return;
  }

  std::string request;
  request.reserve(1024);

  JsonWriter writer(request);
  writer.Write("type", "unregister_command");
  writer.Write("command_id", command->m_id);
  writer.Finalize();

  Send(m_rest_connection, request.c_str(),
       static_cast<unsigned int>(request.size()));
}

void RestClient::ProcessRecv(const ConnectionNotification &notif) {
  if(m_rest_connection != notif.handle) {
    // not our connection, so ignore.
    return;
  }

  SimplePlugin::Recv(m_rest_connection, m_recv_buffer, kRecvBufferSize,
                     [this](void *buffer, unsigned int nbytes) {
                       ProcessJson(buffer, nbytes);
                     });
}

void RestClient::ProcessJson(void *data, unsigned int nbytes) {
  JsonReader reader;
  bool ok = reader.Parse(static_cast<const char *>(data), nbytes);
  if(!ok) {
    PLUGIN_ERROR("failed to parse json document: %s", data);
    return;
  }

  std::string type;
  ok = reader.Read("type", &type);
  if(!ok) {
    PLUGIN_ERROR("json document did not have a 'type' key");
    return;
  }

  if(0 == Base::String::Compare("register_command_response", type.c_str())) {
    ProcessRegisterCommandResponse(reader);
  } else if(0 == Base::String::Compare("process_request", type.c_str())) {
    ProcessRequest(reader);
  } // else an unsupported type was specified
}

void RestClient::ProcessRegisterCommandResponse(JsonReader &reader) {
  bool ok = true;
  std::string command_name;
  int command_id = 0;

  ok &= reader.Read("command_name", &command_name);
  ok &= reader.Read("command_id", &command_id);
  if(ok) {
    PLUGIN_DEBUG("confirmed rest command registration for %s. id %d.",
                 command_name.c_str(), command_id);

    RestCommand *cmd = m_registered;
    while(cmd) {
      if(0 == Base::String::Compare(command_name.c_str(), cmd->Name())) {
        m_commands[command_id] = cmd;
        PLUGIN_INFO("command %s registration succeeded with id %d", cmd->Name(),
                    command_id);
        break;
      }
      cmd = cmd->m_next;
    }
  } else {
    std::string error_string;
    ok = reader.Read("error", &error_string);
    PLUGIN_ERROR("command registration failed with error(%s)",
                 error_string.c_str());
  }
}

void RestClient::ProcessRequest(JsonReader &reader) {
  bool ok = true;
  int command_id = 0;
  int request_id = 0;
  std::string query_string;
  std::string post_data;

  ok = ok && reader.Read("command_id", &command_id);
  ok = ok && reader.Read("request_id", &request_id);
  query_string = reader.Read("query_string", std::string());
  post_data = reader.Read("data", std::string());
  if(!ok) {
    PLUGIN_ERROR("failed to read one or more required request elements.");
    return;
  }

  CommandMap::iterator it = m_commands.find(command_id);
  if(m_commands.end() == it) {
    PLUGIN_ERROR("unable to process unregistered command %d.", command_id);
    return;
  }

  RestCommand *command = it->second;
  std::string response_json;
  ok = command->OnCommand(query_string, post_data, &response_json);

  PLUGIN_INFO("rest command %s - result %s", command->Name(),
              ok ? "success" : "failed");

  std::string response;
  response.reserve(1024 + response_json.size());
  JsonWriter writer(response);
  writer.Write("type", "response_data");
  writer.Write("command_id", command_id);
  writer.Write("request_id", request_id);

  if(!ok) {
    writer.WriteRaw("data", "{\"error\": \"Command processing failed!\"}");
  } else if(response_json.empty()) {
    writer.WriteRaw("data", "{\"error\": \"Supplied invalid response JSON!\"}");
  } else {
    writer.WriteRaw("data", response_json);
  }

  writer.Finalize();

  int send_result = Send(m_rest_connection, response.c_str(),
                         static_cast<unsigned int>(response.size()));
  if(send_result != 0) {
    PLUGIN_ERROR("failed to send command response(%s)", response.c_str());
  }
}

} // namespace Link
