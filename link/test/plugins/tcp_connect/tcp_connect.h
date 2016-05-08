/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "plugin_interface.h"
#include "link/link.h"
#include "plugin/rest/rest_client.h"
#include "plugin/rest/rest_command.h"


class RegisterCmd : public Link::RestCommand {
public:
	RegisterCmd(){}
	const char* Name() const { return "register"; }
	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data);
};

class ConnectCmd : public Link::RestCommand {
public:
	ConnectCmd(ConnectionHandle& handle, bool& est) : m_connection(handle), m_established(est){}
	const char* Name() const { return "connect"; }
	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data);
	ConnectionHandle& m_connection;
	bool& m_established;
};

class TcpConnect : public SimplePlugin, public Link::RestClient {
public:
	TcpConnect();
	bool OnStartup(const char* config, streamsize nbytes);
	void OnShutdown();
	void OnNotification(const Notification& notification);
	void OnConnected(const ConnectionNotification& notification);
	void OnPluginConnected(const ConnectionNotification& notification);
	void OnDisconnected(const ConnectionNotification& notification);
	void OnRecvReady(const ConnectionNotification& notification);
	void OnUpdate(unsigned int dt);

	enum CONSTANT {
		kUpdateDeltaMs = 10
	};
private:
	RegisterCmd m_register_cmd;
	ConnectCmd m_connect_cmd;

	ConnectionHandle m_connection_send;
	ConnectionHandle m_connection_recv;
	int m_num_chunk_send;
	int m_num_chunk_recv;
	bool m_established;
};
