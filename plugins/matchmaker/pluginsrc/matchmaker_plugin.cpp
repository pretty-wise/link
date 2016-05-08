/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "matchmaker_plugin.h"

#include "link/plugin_log.h"

#include "tinyxml2.h"

#include "common/protobuf_stream.h"
#include "common/json/json_writer.h"
//#include "protocol/gate.pb.h"

namespace Link {
namespace Matchmaker {

MatchmakerPlugin::MatchmakerPlugin()
: SimplePlugin(kUpdateDeltaMs) {
	m_recv_buffer = malloc(kRecvBufferSize);
}

MatchmakerPlugin::~MatchmakerPlugin() {
	free(m_recv_buffer);
}

bool MatchmakerPlugin::OnStartup(const char* config, streamsize nbytes) {	
	if(!config || nbytes == 0) {
		PLUGIN_ERROR("no config");
		return false;
	}

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError err = doc.Parse(config, nbytes);

	if(err != tinyxml2::XML_SUCCESS){
		PLUGIN_ERROR("problem parsing config: %s(%d)", doc.ErrorName(), err);
		return false;
	}
/*
	u16 port = 0;

	tinyxml2::XMLElement* root	= doc.RootElement();
	if(root->Attribute("port")) {
		port = root->IntAttribute("port");
		PLUGIN_INFO("port read %d", port);
	} else { 
		PLUGIN_WARN("no port specified, defaulting to 0");
	}
	
	if(!root->Attribute("max_connections")) {
		PLUGIN_ERROR("maximum connection count not specified");
		return false;
	}
	u32 max_connections = root->IntAttribute("max_connections");
	PLUGIN_INFO("maximum number of connections: %d", max_connections);
*/
	return true;
}

void MatchmakerPlugin::OnShutdown() {
}

void MatchmakerPlugin::OnUpdate(unsigned int dt) { 
	(void)dt; 
}

void MatchmakerPlugin::OnRecvReady(const ConnectionNotification& notif) {
	SimplePlugin::Recv(notif.handle, m_recv_buffer, kRecvBufferSize, [&](void* buffer, unsigned int nbytes){
		ParseDataReceived(buffer, nbytes, notif.handle, notif.endpoint);
	});
}

void MatchmakerPlugin::OnNotification(const Notification& notif) {
	ProcessNotification(notif);
}

void MatchmakerPlugin::OnPluginConnected(const ConnectionNotification& notif) {
	if(GetRestConnection() == notif.handle) {
		return; // ignore rest plugin connection.
	}
}

void MatchmakerPlugin::OnConnected(const ConnectionNotification& notif) {
	if(GetRestConnection() == notif.handle) {
		return; // ignore rest plugin connection.
	}
}

void MatchmakerPlugin::OnDisconnected(const ConnectionNotification& notif) {
	if(GetRestConnection() == notif.handle) {
		return; // ignore rest plugin connection.
	}
}

void MatchmakerPlugin::ParseDataReceived(void* buffer, unsigned int nbytes, ConnectionHandle connection, PluginHandle plugin) {
}
} // namespace Matchmaker 
} // namespace Link

const char* SimplePlugin::Name = "match";
const char* SimplePlugin::Version = "0.1";

SimplePlugin* SimplePlugin::CreatePlugin() {
	return new Link::Matchmaker::MatchmakerPlugin();
}

void SimplePlugin::DestroyPlugin(SimplePlugin* plugin) {
	delete plugin;
}
