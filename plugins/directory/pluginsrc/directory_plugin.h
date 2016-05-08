/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "plugin_interface.h"
#include "plugin/rest/rest_client.h"
#include "plugin/rest/rest_command.h"

#include "json_utils.h"

#include <list>
#include <map>

const char* SimplePlugin::Name = "directory";
const char* SimplePlugin::Version = "0.1";

namespace Link {

class DirectoryPlugin;

class ListPluginsCmd : public RestCommand {
public:
	ListPluginsCmd(DirectoryPlugin& dir) : m_directory(dir){}
	const char* Name() const { return "plugin-list"; }
	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data);
private:
	DirectoryPlugin& m_directory;
};

/* Directory plugin keeps track of availability of all plugins in the running process. 
	Directory plugin can be run in one of two modes:
		* Master mode.
		* Slave mode, when it connects to other directory plugin which runs in 'master mode'.
	It registers the master directory plugin specified in the config file 
	and tries to establish a connection with the master plugin.

	Slave plugin can:
	- publish a plugin
	- suppress a plugin
	Master plugin will:
	- announce plugins to all slaves
	- revoke plugins to all slaves.

	When slave plugin connects to master it announces all his local plugins.
	When master plugin gets notified about slave connection it announces all plugins to slave.
	When local plugin becomes available or unavailable suppress or announce message is sent to master plugin. The master
	plugin broadcasts this message to all slaves.
*/
class DirectoryPlugin : public SimplePlugin, public RestClient {
public:
	DirectoryPlugin();
	~DirectoryPlugin();
	bool OnStartup(const char* config, streamsize nbytes);
	void OnShutdown();

	void OnUpdate(unsigned int dt);
	void OnWatchMatch(const WatchNotification& notification);
	void OnNotification(const Notification& notif);
	void OnRecvReady(const ConnectionNotification& notif);
	void OnPluginConnected(const ConnectionNotification& notification);
	void OnConnected(const ConnectionNotification& notification);
	void OnDisconnected(const ConnectionNotification& notification);

	enum CONSTANTS {
		kUpdateDeltaMs = 10,
		kRecvBufferSize = 1024
	};

	void WritePluginList(std::string* data);
private:
	typedef std::list<PluginHandle> PluginList;
	typedef std::map<ConnectionHandle, PluginList> ConnectionToPlugins;

	bool RegisterMasterPlugin(const char* name, const char* version, const char* hostname, u16 port, int pid);

	void AnnounceAll(ConnectionHandle conn);
	void AnnounceToAll(PluginHandle plugin, const PluginInfo& info);
	void PublishLocalPlugins();
	void Revoke(ConnectionHandle conn, const PluginList& plugins);
	void RevokeToAll(PluginHandle plugin);
	void Announce(PluginHandle plugin, ConnectionHandle conn);
	void Announce(PluginHandle plugin, const PluginInfo& info, ConnectionHandle conn);
	void Publish(PluginHandle plugin);
	void Publish(PluginHandle plugin, const PluginInfo& info);
	void Suppress(PluginHandle plugin);
	void Revoke(PluginHandle plugin, ConnectionHandle conn);

	void AddPluginToRegistry(ConnectionHandle conn, PluginHandle plugin, const PluginInfo& info);

	bool m_is_master;
	PluginInfo m_master_info; // master plugin info.
	PluginHandle m_master_handle; // master plugin handle.
	ConnectionHandle m_master_connection; // slave to master connection.

	void* m_recv_buffer;

	//! Watch for local plugins.
	WatchHandle m_watch;

	struct ConnectionData {
		ConnectionHandle connection;
		PluginHandle plugin;
	};

	ConnectionToPlugins m_connection_to_plugin_map;
	//std::vector<ConnectionData> m_connection;

	ListPluginsCmd m_plugin_list_cmd;
};

} // namespace Link

SimplePlugin* SimplePlugin::CreatePlugin() {
	return new Link::DirectoryPlugin();
}

void SimplePlugin::DestroyPlugin(SimplePlugin* plugin) {
	delete plugin;
}
