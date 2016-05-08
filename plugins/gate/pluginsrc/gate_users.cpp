/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gate_users.h"
#include "link/plugin_log.h"
#include "base/core/assert.h"
#include "gate_commands.h"
#include "common/json/json_writer.h"

#include <cstring>

namespace Link {
namespace Gate {

Connection::Connection() : m_handle(TCPServer::kInvalidHandle) {
  for(u32 i = 0; i < Gate::kMaxUsersPerConnection; ++i) {
    m_user_handle[i] = kInvalidUserHandle;
  }
}

Connection::Connection(TCPServer::Handle handle, const Base::Url &address)
    : m_handle(handle), m_address(address) {
  for(u32 i = 0; i < Gate::kMaxUsersPerConnection; ++i) {
    m_user_handle[i] = kInvalidUserHandle;
  }
}

u32 Connection::GetUserCount() const {
  u32 count = 0;
  for(u32 i = 0; i < Gate::kMaxUsersPerConnection; ++i) {
    if(m_user_handle[i] != kInvalidUserHandle) {
      ++count;
    }
  }
  return count;
}

void Connection::AddUser(index_t index) {
  for(u32 i = 0; i < Gate::kMaxUsersPerConnection; ++i) {
    if(m_user_handle[i] == kInvalidUserHandle) {
      m_user_handle[i] = index;
      return;
    }
  }
}

void Connection::RemoveUser(index_t index) {
  for(u32 i = 0; i < Gate::kMaxUsersPerConnection; ++i) {
    if(m_user_handle[i] == index) {
      m_user_handle[i] = kInvalidUserHandle;
    }
  }
}

Users::Users(u32 max_connections) : m_max_connections(max_connections) {
  u32 user_count = max_connections * Gate::kMaxUsersPerConnection;
  m_connections = new Connection[m_max_connections];
  m_users = new user_t[user_count];
  m_next_user = 0;

  if(m_connections == nullptr || m_users == nullptr) {
    PLUGIN_ERROR("out of memory");
    delete[] m_connections;
    delete[] m_users;
    return;
  }
}

Users::~Users() {
  delete[] m_connections;
  delete[] m_users;
}

void Users::AddConnection(TCPServer::Handle handle, const Base::Url &address) {
  Connection &new_conn = m_connections[handle];
  BASE_ASSERT(new_conn.GetHandle() == TCPServer::kInvalidHandle);
  new_conn = Connection(handle, address);
}

void Users::RemConnection(TCPServer::Handle handle) {
  Connection &conn = m_connections[handle];
  BASE_ASSERT(conn.GetHandle() == handle);
  conn = Connection();
}

Connection &Users::GetConnection(TCPServer::Handle handle) {
  BASE_ASSERT(handle < m_max_connections);
  Connection &conn = m_connections[handle];
  BASE_ASSERT(conn.GetHandle() == handle);
  return conn;
}

Connection &Users::GetConnectionByUserId(index_t user) {
  BASE_ASSERT(user >= m_max_connections * kMaxUsersPerConnection);
  BASE_ASSERT(m_users[user].connection != TCPServer::kInvalidHandle);
  return m_connections[m_users[user].connection];
}

bool Users::IsConnected(TCPServer::Handle handle) const {
  if(handle >= m_max_connections) {
    return false;
  }

  if(m_connections[handle].GetHandle() != handle) {
    return false;
  }
  return true;
}

index_t Users::AddUser(TCPServer::Handle handle, const char *username) {
  BASE_ASSERT(handle < m_max_connections);
  Connection &conn = m_connections[handle];
  if(conn.GetUserCount() >= kMaxUsersPerConnection) {
    PLUGIN_ERROR("cannot login more then %d users", kMaxUsersPerConnection);
    return kInvalidUserHandle;
  }

  size_t username_length = strlen(username);
  if(username_length > kUsernameMax) {
    PLUGIN_ERROR("cannot login - username too long '%s'", username);
    return kInvalidUserHandle;
  }

  index_t user_index = m_next_user;
  while(m_users[user_index].connection != TCPServer::kInvalidHandle) {
    ++user_index;
    if(user_index == m_max_connections) {
      user_index = 0;
    }
    if(user_index == m_next_user) {
      // this should never happen as we reserve space for kMaxUsersPerConnection
      PLUGIN_ERROR("cannot login anymore users");
      return kInvalidUserHandle;
    }
  }

  m_next_user = user_index + 1;
  if(m_next_user >= m_max_connections) {
    m_next_user = 0;
  }

  user_t &user = m_users[user_index];
  strncpy(user.data.username, username, username_length);
  user.data.username[username_length] = '\0';
  user.connection = handle;

  PLUGIN_INFO("adding user '%s' for connection %d at index %d", username,
              handle, user_index);

  conn.AddUser(user_index);
  return user_index;
}

bool Users::RemUser(TCPServer::Handle handle, index_t index) {
  if(index >= m_max_connections * kMaxUsersPerConnection) {
    PLUGIN_ERROR("problem removing user - index out of bounds");
    return false;
  }

  user_t &user = m_users[index];

  if(user.connection != handle) {
    PLUGIN_ERROR("problem removing user - wrong connection: %d, actual %d",
                 user.connection, handle);
    return false;
  }

  Connection &conn = m_connections[user.connection];
  PLUGIN_INFO("removed user at %d from connection %d", index, user.connection);

  conn.RemoveUser(index);
  user.connection = TCPServer::kInvalidHandle;
  memset(user.data.username, 0, kUsernameMax);

  return true;
}

bool Users::IsLoggedIn(index_t user_id) const {
  if(user_id >= m_max_connections * kMaxUsersPerConnection) {
    return false;
  }
  user_t &user = m_users[user_id];
  TCPServer::Handle handle = user.connection;
  if(handle == TCPServer::kInvalidHandle) {
    return false;
  }
  return true;
}

void Users::Write(std::string *data) const {
  JsonWriter writer(*data);

  std::vector<Gate::ListUsersCmd::Connection> connections;

  for(u32 conn_index = 0; conn_index < m_max_connections; ++conn_index) {
    Connection &conn = m_connections[conn_index];
    if(conn.GetHandle() != TCPServer::kInvalidHandle) {
      Gate::ListUsersCmd::Connection list_connection;
      list_connection.id = conn.GetHandle();
      list_connection.url = conn.GetUrl();
      for(u32 user_it = 0; user_it < kMaxUsersPerConnection; ++user_it) {
        index_t user_index = conn.GetUserHandle(user_it);
        if(user_index != kInvalidUserHandle) {
          user_t &user = m_users[user_index];
          Gate::ListUsersCmd::User list_user;
          list_user.id = user_index;
          PLUGIN_INFO("adding user");
          strncpy(list_user.username, user.data.username, kUsernameMax);
          list_connection.users.push_back(list_user);
        }
      }
      connections.push_back(list_connection);
    }
  }
  writer.Write("connections", connections);
}

} // namespace Gate
} // namespace Link
