/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include "plugin/gate/gate_defs.h"
#include "common/tcp_server.h"
#include "base/core/assert.h"

namespace Link {
namespace Gate {

typedef u32 index_t;
static const index_t kInvalidUserHandle = (index_t)-1;

struct user_data_t {
  char username[Gate::kUsernameMax + 1];
};

class Connection {
public:
  Connection();
  Connection(TCPServer::Handle handle, const Base::Url &address);

  TCPServer::Handle GetHandle() const { return m_handle; }
  const Base::Url &GetUrl() const { return m_address; }

  u32 GetUserCount() const;
  index_t GetUserHandle(u32 i) {
    BASE_ASSERT(i < Gate::kMaxUsersPerConnection);
    return m_user_handle[i];
  }
  void AddUser(index_t index);
  void RemoveUser(index_t index);

private:
  TCPServer::Handle m_handle;
  Base::Url m_address;
  index_t m_user_handle[Gate::kMaxUsersPerConnection];
};

struct user_t {
  user_t() : connection(TCPServer::kInvalidHandle) {}
  TCPServer::Handle connection;
  user_data_t data;
};

class Users {
public:
  Users(u32 max_connections);
  ~Users();

  void AddConnection(TCPServer::Handle handle, const Base::Url &address);
  void RemConnection(TCPServer::Handle handle);
  bool IsConnected(TCPServer::Handle handle) const;
  Connection &GetConnection(TCPServer::Handle handle);
  Connection &GetConnectionByUserId(index_t user);

  index_t AddUser(TCPServer::Handle conn, const char *username);
  bool RemUser(TCPServer::Handle conn, index_t index);
  bool IsLoggedIn(index_t user) const;

  void Write(std::string *data) const;

private:
  Connection *m_connections;
  user_t *m_users;
  index_t m_next_user;
  index_t m_max_connections;
};

} // namespace Gate
} // namespace Link
