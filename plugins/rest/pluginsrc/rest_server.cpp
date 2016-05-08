/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "rest_server.h"

#include "base/core/assert.h"
#include "base/core/time_utils.h"

#include "base/math/crc.h"
#include "link/plugin_log.h"
#include "common/json/json_writer.h"

#include <atomic>

namespace Link {

int begin_request(struct mg_connection *conn) {
  PLUGIN_INFO("civetweb: begin_request %p", conn);
  return 0;
}

void end_request(const struct mg_connection *conn, int reply_status_code) {
  PLUGIN_INFO("civetweb: end_request %p with: %d.", conn, reply_status_code);
}

int log_message(const struct mg_connection *conn, const char *message) {
  PLUGIN_INFO("civetweb: log(%p): %s.", conn, message);
  return 1; // non-zero means the log message has been served.
}

void connection_closed(struct mg_connection *conn) {
  PLUGIN_INFO("civetweb: connection_closed %p", conn);
}

int http_error(struct mg_connection *conn, int status, const char *reason) {
  PLUGIN_ERROR("civetweb: http_error %p with: %d. reason: %s.", conn, status,
               reason);
  return 0; // zero means the error will be sent by civet. we only log this
            // here.
}

RestServer::RestServer() : m_context(nullptr) {
  m_commands_lock.Initialize();
  m_connections_lock.Initialize();
  m_timeout_lock.Initialize();
}

RestServer::~RestServer() {
  BASE_ASSERT(m_context == nullptr);

  m_commands_lock.Terminate();
  m_connections_lock.Terminate();
  m_timeout_lock.Terminate();
}

bool RestServer::Start(const char **options) {
  if(m_context) {
    PLUGIN_WARN("context already exists");
    return false; // already running.
  }

  struct mg_callbacks callbacks = {
      begin_request,     end_request, log_message, 0, 0,         0, 0,
      connection_closed, 0,           0,           0, http_error};

  m_context = mg_start(&callbacks, this, options);

  if(!m_context) {
    PLUGIN_ERROR("failed to create context");
    return false;
  }

  // list server listen ports.
  const size_t kMaxPorts = 16;
  int port_num[kMaxPorts];
  int is_ssl[kMaxPorts];
  size_t n_ports = mg_get_ports(m_context, kMaxPorts, port_num, is_ssl);
  for(size_t i = 0; i < n_ports; ++i) {
    PLUGIN_INFO("rest server listening on %sport: %d", is_ssl[i] ? "SSL " : "",
                port_num[i]);
  }

  return true;
}

void RestServer::Stop() {
  if(m_context) {
    mg_stop(m_context);
  }
  m_context = nullptr;
}

bool RestServer::Register(const char *cmd_uri, ConnectionHandle connection) {
  if(!cmd_uri || cmd_uri[0] == '\0') {
    // invalid command name.
    return false;
  }

  int command_id = Base::Math::crc(cmd_uri);

  Command command_data;
  if(Find(command_id, &command_data)) {
    // command name already registered.
    return false;
  }

  command_data.handle = connection;
  command_data.uri = cmd_uri;

  {
    Base::MutexAutoLock lock(m_commands_lock);
    m_commands[command_id] = command_data;
  }

  mg_set_request_handler(m_context, cmd_uri, RequestHandler,
                         reinterpret_cast<void *>(command_id));
  PLUGIN_INFO("registered command %s as command id %d.", cmd_uri, command_id);

  return true;
}

void RestServer::Unregister(const char *cmd_uri) {
  int command_id = Base::Math::crc(cmd_uri);
  Unregister(command_id);
}

void RestServer::Unregister(int command_id) {
  Base::MutexAutoLock lock(m_commands_lock);
  CommandList::iterator it = m_commands.find(command_id);
  if(it != m_commands.end()) {
    mg_set_request_handler(m_context, (*it).second.uri.c_str(), NULL, NULL);
    PLUGIN_INFO("unregistered command %s, command id: %d",
                (*it).second.uri.c_str(), command_id);
    m_commands.erase(command_id);
  }
}

void RestServer::UnregisterAll(ConnectionHandle connection) {
  Base::MutexAutoLock lock(m_commands_lock);
  for(CommandList::iterator it = m_commands.begin(); it != m_commands.end();) {
    if((*it).second.handle == connection) {
      mg_set_request_handler(m_context, (*it).second.uri.c_str(), NULL, NULL);
      it = m_commands.erase(it);
    } else {
      ++it;
    }
  }
}

bool RestServer::SendResponse(ConnectionHandle connection_handle,
                              int request_id, const char *bytes, int length) {
  bool success = false;
  Connection conn_data;

  {
    Base::MutexAutoLock lock(m_connections_lock);

    ConnectionList::iterator it = m_connections.find(request_id);

    if(it == m_connections.end()) {
      return false;
    }
    conn_data = it->second;

    BASE_ASSERT(conn_data.handle == connection_handle);

    // write response headers.
    success =
        0 <
        mg_printf(
            conn_data.connection,
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: %d\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Headers: Origin, X-Requested-With, "
            "Content-Type, Accept\r\n"
            //												"Date:
            //%s\r\n"
            "Connection: close\r\n\r\n",
            length);

    // write response content.
    success |= mg_write(conn_data.connection, bytes, length) > 0;

    PLUGIN_INFO("forwarding response from plugin %p for request %d %s.",
                connection_handle, request_id, success ? "succeded" : "failed");

    mg_close_connection(conn_data.connection);

    m_connections.erase(it);
  }
  return success;
}

int RestServer::RequestHandler(struct mg_connection *conn, void *cbdata) {
  struct mg_request_info *request_info = mg_get_request_info(conn);
  RestServer *_this = static_cast<RestServer *>(request_info->user_data);
  BASE_ASSERT(request_info != NULL);
  BASE_ASSERT(_this != NULL);

  // casting to avoid strict aliasing!
  int command_id = static_cast<int>(reinterpret_cast<uintptr_t>(cbdata));

  Command cmd;
  bool cmd_found = _this->Find(command_id, &cmd);

  if(!cmd_found) {
    PLUGIN_ERROR("no handler found for command %d.", command_id);
    return 0; // No handler found
  }

  const char *query = "";
  char *post_data = NULL;

  // get request data.
  if(Base::String::Compare(request_info->request_method, "GET") == 0) {
    query = mg_get_request_info(conn)->query_string
                ? mg_get_request_info(conn)->query_string
                : "";
  } else if(Base::String::Compare(request_info->request_method, "POST") == 0) {
    query = mg_get_request_info(conn)->query_string
                ? mg_get_request_info(conn)->query_string
                : "";

    const char *con_len_str = mg_get_header(conn, "Content-Length");
    int con_len = 0;

    if(con_len_str) {
      con_len = atoi(con_len_str);
    }

    if(con_len > 0) {
      // TODO: we might need to optimize this. remember: this code is executed
      // in parallel to other incoming requests.
      post_data = (char *)malloc(con_len + 1);

      int read = mg_read(conn, post_data, con_len);

      if(read == con_len) {
        post_data[read] = '\0'; // null terminate
      }
    }
  }

  static std::atomic_int s_request_id(0);
  signed int request_id = s_request_id++;

  std::string output;

  JsonWriter writer(output);
  writer.Write("type", "process_request");
  writer.Write("command_id", command_id);
  writer.Write("request_id", request_id);
  writer.Write("query_string", query);
  writer.Write<const char *>("data", post_data ? post_data : "");
  writer.Finalize();

  bool result = _this->ForwardToPlugin(cmd.handle, output.c_str(),
                                       (unsigned int)output.length(),
                                       request_id, conn, kRequestTimeout);

  PLUGIN_INFO(
      "forwarding request to plugin %p command id: %d, request id: %d %s.",
      cmd.handle, command_id, request_id, result ? "succeeded" : "failed");

  free(post_data);

  return result ? 1 : 0;
}

bool RestServer::ForwardToPlugin(ConnectionHandle plugin_conn_handle,
                                 const char *message, unsigned int length,
                                 int request_id, struct mg_connection *conn,
                                 unsigned int timeout_ms) {
  // need to lock! BASE_ASSERT(m_connections.find(request_id) ==
  // m_connections.end());

  // add to connection list.
  Connection connection;
  connection.connection = conn;
  connection.handle = plugin_conn_handle;

  {
    Base::MutexAutoLock lock(m_connections_lock);
    m_connections[request_id] = connection;
  }

  // add to timeout list
  Timeout timeout;
  timeout.request_id = request_id;
  timeout.expiration_time = Base::Time::GetTimeMs() + timeout_ms;

  {
    Base::MutexAutoLock lock(m_timeout_lock);
    m_timeout.push(timeout);
  }

  bool sent = 0 == Send(plugin_conn_handle, message, length);

  if(!sent) {
    // failed sending message to a plugin.
    // remove data from Connection list.
    // timeout data will be popped later without any consequence.
    PLUGIN_ERROR("failed forwarding command to plugin");
    Base::MutexAutoLock lock(m_connections_lock);
    m_connections.erase(m_connections.find(request_id));
  }

  return sent;
}

bool RestServer::Find(int command_id, struct Command *cmd) {
  BASE_ASSERT(cmd, "cmd must point to valid command mem");

  Base::MutexAutoLock lock(m_commands_lock);

  CommandList::iterator it = m_commands.find(command_id);

  if(it == m_commands.end()) {
    return false;
  }

  *cmd = it->second;
  return true;
}

void RestServer::WriteAvailableCommands(JsonWriter *writer) const {
  std::vector<std::string> commands;

  Base::MutexAutoLock lock(m_commands_lock);

  for(CommandList::const_iterator it = m_commands.begin();
      it != m_commands.end(); ++it) {
    commands.push_back((*it).second.uri);
  }
  writer->Write("commands", commands);
}

} // namespace Link
