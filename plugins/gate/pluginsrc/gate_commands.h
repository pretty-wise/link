/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once 

#include "plugin/rest/rest_command.h"
#include "common/json/json_writer.h"
#include "base/network/url.h"
#include "plugin/gate/gate_defs.h"

namespace Link {
namespace Gate {

class GatePlugin;

class ListUsersCmd : public RestCommand {
public:
	ListUsersCmd(GatePlugin& plugin) : m_plugin(plugin) {}
	virtual const char* Name() const { return "list_users"; }
	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data);

	struct User {
		int id;
		char username[Gate::kUsernameMax];
	};
	struct Connection {
		int id;
		Base::Url url;
		std::vector<User> users;
	};
private:
	GatePlugin& m_plugin;
};

class DisconnectCmd : public RestCommand {
public:
	DisconnectCmd(GatePlugin& plugin) : m_plugin(plugin) {}
	virtual const char* Name() const { return "disconnect"; }
	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data);
private:
	GatePlugin& m_plugin;
};

class LogoutCmd : public RestCommand {
public:
	LogoutCmd(GatePlugin& plugin) : m_plugin(plugin) {}
	virtual const char* Name() const { return "logout"; }
	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data);
private:
	GatePlugin& m_plugin;
};

} // namespace Gate

template<>
inline void JsonWriter::AppendValue(const Gate::ListUsersCmd::User& user) {
	m_write_count = 0;
	m_destination += "{";

	Write("id", user.id);
	Write("username", user.username);

	m_write_count = 0;
	m_destination += "}";
}

template<>
inline void JsonWriter::AppendValue(const Gate::ListUsersCmd::Connection& info) {
	m_write_count = 0;
	m_destination += "{";

	Write("id", info.id);
	const u32 addr_buf_len = 256;
	char address_buffer[addr_buf_len];
	snprintf(address_buffer, addr_buf_len, "%d.%d.%d.%d:%d", PRINTF_URL(info.url));
	Write("url", address_buffer);
	Write("users", info.users);	

	m_write_count = 0;
	m_destination += "}";
}

} // namespace Link
