/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "plugin_interface.h"
#include "rest_server.h"
#include "plugin/rest/rest_client.h"
#include "plugin/rest/rest_command.h"

const char* SimplePlugin::Name = "rest";
const char* SimplePlugin::Version = "0.1";

namespace Link {

class CommandListCmd : public RestCommand {
public:
	CommandListCmd(RestServer& server) : m_server(server){}
	const char* Name() const { return "command-list"; }
	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data);
private:
	RestServer& m_server;
};

class RestPlugin : public SimplePlugin, public RestClient {
public:
	RestPlugin();
	~RestPlugin();
	bool OnStartup(const char* config, streamsize nbytes);
	void OnShutdown();

	void OnNotification(const Notification& notif);
	void OnRecvReady(const ConnectionNotification& notif);

	enum CONSTANTS {
		kUpdateDeltaMs = 10,
		kRecvBufferSize = 1024
	};
private:
	void ParseDataReceived(void* buffer, unsigned int nbytes, ConnectionHandle connection, PluginHandle plugin);
	void OnRegisterCommand(const char* command_name, ConnectionHandle connection, PluginHandle plugin);
	void OnUnregisterCommand(int command_id, ConnectionHandle handle);
	void OnResponseData(int request_id, const char* response_data, int response_length, ConnectionHandle handle);
	std::string GetFullCommandName(const char* cmd_name, const PluginInfo& info);

	RestServer m_server;
	CommandListCmd m_command_list;
	void* m_recv_buffer;
};

} // namespace Link


SimplePlugin* SimplePlugin::CreatePlugin() {
	return new Link::RestPlugin();
}

void SimplePlugin::DestroyPlugin(SimplePlugin* plugin) {
	delete plugin;
}
