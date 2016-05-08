/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "punch_plugin.h"

#include "link/plugin_log.h"
#include "base/network/url.h"

#include "tinyxml2.h"

#include "common/protobuf_stream.h"
#include "common/json/json_writer.h"

namespace Link {
namespace Punch {

PunchPlugin::PunchPlugin()
: SimplePlugin(kUpdateDeltaMs) {
	m_recv_buffer = malloc(kRecvBufferSize);
}

PunchPlugin::~PunchPlugin() {
	free(m_recv_buffer);
}

bool PunchPlugin::OnStartup(const char* config, streamsize nbytes) {
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
	u16 port = 0;

	tinyxml2::XMLElement* root	= doc.RootElement();
	if(root->Attribute("port")) {
		port = root->IntAttribute("port");
		PLUGIN_INFO("port read %d", port);
	} else {
		PLUGIN_WARN("no port specified, defaulting to 0");
	}

	std::string hostname;
	if(root->Attribute("hostname")) {
		hostname = root->Attribute("hostname");
		PLUGIN_INFO("hostname read %s", hostname.c_str());
	}
	Base::AddressIPv4 address;
	if(!Base::AddressIPv4::FromString(&address, hostname.c_str())) {
		PLUGIN_ERROR("problem reading hostname");
		return false;
	}

	u32 addr = address.GetRaw();
	m_punch = punch_server_create(addr, &port);
	Base::Url url(addr, port);
	PLUGIN_INFO("punch server started at: %d.%d.%d.%d:%d", PRINTF_URL(url));
	return m_punch != nullptr;
}

void PunchPlugin::OnShutdown() {
	punch_server_destroy(m_punch);
}

void PunchPlugin::OnUpdate(unsigned int dt) {
	(void)dt;
	punch_server_update(m_punch, dt);
}

void PunchPlugin::OnRecvReady(const ConnectionNotification& notif) {
	SimplePlugin::Recv(notif.handle, m_recv_buffer, kRecvBufferSize, [&](void* buffer, unsigned int nbytes){
		ParseDataReceived(buffer, nbytes, notif.handle, notif.endpoint);
	});
}

void PunchPlugin::OnNotification(const Notification& notif) {
	ProcessNotification(notif);
}

void PunchPlugin::OnPluginConnected(const ConnectionNotification& notif) {
	if(GetRestConnection() == notif.handle) {
		return; // ignore rest plugin connection.
	}
}

void PunchPlugin::OnConnected(const ConnectionNotification& notif) {
	if(GetRestConnection() == notif.handle) {
		return; // ignore rest plugin connection.
	}
}

void PunchPlugin::OnDisconnected(const ConnectionNotification& notif) {
	if(GetRestConnection() == notif.handle) {
		return; // ignore rest plugin connection.
	}
}

void PunchPlugin::ParseDataReceived(void* buffer, unsigned int nbytes, ConnectionHandle connection, PluginHandle plugin) {
}
} // namespace Punch
} // namespace Link

const char* SimplePlugin::Name = "punch";
const char* SimplePlugin::Version = "0.1";

SimplePlugin* SimplePlugin::CreatePlugin() {
	return new Link::Punch::PunchPlugin();
}

void SimplePlugin::DestroyPlugin(SimplePlugin* plugin) {
	delete plugin;
}
