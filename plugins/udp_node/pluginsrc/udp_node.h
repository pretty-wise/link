/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include "plugin_interface.h"
#include "plugin/rest/rest_client.h"
#include "plugin/rest/rest_command.h"
#include "rush/rush.h"

#include <vector>

namespace Link {

class UdpNode;

class InfoCmd : public RestCommand {
public:
	InfoCmd(rush_t& ctx, UdpNode& node) : m_ctx(ctx), m_plugin(node){}
	const char* Name() const { return "info"; }

	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data);
private:
	rush_t& m_ctx;
	UdpNode& m_plugin;
};

class ConnectCmd : public RestCommand {
public:
	ConnectCmd(rush_t& ctx) : m_ctx(ctx){}
	const char* Name() const { return "connect"; }

	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data);
private:
	rush_t& m_ctx;
};

class DisconnectCmd : public RestCommand {
public:
	DisconnectCmd(rush_t& ctx, UdpNode& node) : m_ctx(ctx), m_plugin(node){}
	const char* Name() const { return "disconnect"; }

	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data);
private:
	rush_t& m_ctx;
	UdpNode& m_plugin;
};

class ListConnectionsCmd : public RestCommand {
public:
	ListConnectionsCmd(rush_t& ctx) : m_ctx(ctx) {}
	const char* Name() const { return "list-connections"; }
	virtual bool OnCommand(const std::string& query_string,
												const std::string& post_data,
												std::string* response_data);
private:
	rush_t& m_ctx;
};

class RegulationCmd : public RestCommand {
public:
	RegulationCmd(rush_t& ctx) : m_ctx(ctx) {}
	const char* Name() const { return "regulation"; }
	virtual bool OnCommand(const std::string& query_string,
												const std::string& post_data,
												std::string* response_data);
private:
	rush_t& m_ctx;
};

class UdpNode : public SimplePlugin, public RestClient {
public:
	UdpNode();
	~UdpNode();
	bool OnStartup(const char* config, streamsize nbytes);
	void OnShutdown();

	void OnNotification(const Notification& notif);
	void OnRecvReady(const ConnectionNotification& notif);
	void OnUpdate(unsigned int dt);
 
	enum CONSTANTS {
		kUpdateDeltaMs = 10,
		kRecvBufferSize = 1024
	};

	std::vector<endpoint_t> connection;
private:
	void* m_recv_buffer;

	InfoCmd m_info;
	ConnectCmd m_connect;
	DisconnectCmd m_disconnect;
	ListConnectionsCmd m_list;
	RegulationCmd m_reg;
};

} // namespace Link
