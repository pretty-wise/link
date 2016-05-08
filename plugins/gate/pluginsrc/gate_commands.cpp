/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gate_commands.h"
#include "link/plugin_log.h"
#include "gate_plugin.h"
#include "gate_users.h"

#include <stdlib.h> // atoi
#include <cstring>

namespace Link {
namespace Gate {

template<class T>
bool FindIntArgument(const char* arg, T* result, const std::string& query) {
	size_t arg_len = strlen(arg);
	size_t start = query.find(arg);
	if(start == std::string::npos) {
		return false;
	}
	size_t num_start = start + arg_len + 1;
	if(num_start >= query.length()
		|| query[num_start-1] != '=') {
		return false;
	}

	*result = atoi(&query[num_start]);
	return true;
}

bool ListUsersCmd::OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data) {
	PLUGIN_INFO("list users");
	m_plugin.WriteUserList(response_data);
	return true;
}

bool DisconnectCmd::OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data) {
	PLUGIN_INFO("disconnect %s", query_string.c_str());
	TCPServer::Handle id = 0;
	if(!FindIntArgument<TCPServer::Handle>("id", &id, query_string)) {
		*response_data = "{ \"error\": \"wrong input\" }";
		return true;
	}
	if(!m_plugin.Disconnect(id)) {
		*response_data = "{ \"error\": \"cannot disconnect\" }";
		return true;
	}

	*response_data = "{ }";
	return true;
}

bool LogoutCmd::OnCommand(const std::string& query_string,
												const std::string& post_data,
												std::string* response_data) {
	PLUGIN_INFO("logout %s", query_string.c_str());
	UserId user_id = kInvalidUserId;
	if(!FindIntArgument<UserId>("user", &user_id, query_string)) {
		*response_data = "{ \"error\": \"wrong input\" }";
		return true;
	}
	if(!m_plugin.Logout(user_id)) {
		*response_data = "{ \"error\": \"cannot logout\" }";
		return true;
	}

	*response_data = "{ }";
	return true;
}

} // namespace Gate
} // namespace Link
