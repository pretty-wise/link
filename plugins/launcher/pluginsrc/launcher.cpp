/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "launcher.h"

#include "link/plugin_log.h"
#include "base/core/str.h"
#include "base/math/crc.h"

#include "json_utils.h"
#include "common/json/json_reader.h"
#include "common/json/json_writer.h"
#include "tinyxml2.h"

const char* SimplePlugin::Name = "launcher";
const char* SimplePlugin::Version = "0.1";

SimplePlugin* SimplePlugin::CreatePlugin() {
	return new Link::Launcher();
}

void SimplePlugin::DestroyPlugin(SimplePlugin* plugin) {
	delete plugin;
}

namespace Link {

Launcher::Launcher()
: SimplePlugin(kUpdateDeltaMs)
, m_list(&m_pids)
, m_run(&m_pids)
, m_kill(&m_pids) {
	m_recv_buffer = malloc(kRecvBufferSize);
}

Launcher::~Launcher() {
	free(m_recv_buffer);
}

bool Launcher::OnStartup(const char* config, streamsize nbytes) {
	if(!RegisterCommand(&m_run)) {
		return false;
	}
	if(!RegisterCommand(&m_kill)) {
		return false;
	}
	if(!RegisterCommand(&m_list)) {
		return false;
	}

	return true;
}

void Launcher::OnShutdown() {
	UnregisterCommand(&m_run);
	UnregisterCommand(&m_kill);
	UnregisterCommand(&m_list);
}

void Launcher::OnUpdate(unsigned int dt) {
}

void Launcher::OnRecvReady(const ConnectionNotification& notif) {
	SimplePlugin::Recv(notif.handle, m_recv_buffer, kRecvBufferSize, [&](void* buffer, unsigned int nbytes){
		//ParseDataReceived(buffer, nbytes, notif.handle, notif.endpoint);
	});
}

void Launcher::OnNotification(const Notification& notif) {
	RestClient::ProcessNotification(notif);
}

} // namespace Link
