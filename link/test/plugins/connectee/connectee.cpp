/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "connectee.h"
#include "link/plugin_log.h"
#include "base/core/time_utils.h"

#include <stdlib.h>

Connectee::Connectee()
 : SimplePlugin(kUpdateDeltaMs)
 , m_recv_buffer(nullptr)
 , m_time_connected(0)
 , m_time_disconnected(0)
 , m_bytes_received(0) {
}

bool Connectee::OnStartup(const char* config, streamsize nbytes) {
	m_recv_buffer = malloc(kRecvBufferSize);
	return true;
}

void Connectee::OnShutdown() {

	if(0 == m_time_disconnected) {
		PLUGIN_WARN("disconnection not notified");
		m_time_disconnected = Base::Time::GetTimeMs();
	}

	unsigned int duration_ms = m_time_disconnected - m_time_connected;
	unsigned int duration_sec = duration_ms / 1000;
	u64 kb_received = m_bytes_received / 1000;
	u64 mb_received = kb_received / 1000;

	PLUGIN_INFO("downstream: received %luB in %dms", m_bytes_received, duration_ms);
	PLUGIN_INFO("downstream: %luB/s. %luKB/s. %luMB/s",
			m_bytes_received / duration_sec,
			kb_received / duration_sec,
			mb_received / duration_sec);

	free(m_recv_buffer);
	m_recv_buffer = nullptr;
}

void Connectee::OnConnected(const ConnectionNotification& notification) {
	m_time_connected = Base::Time::GetTimeMs();
	PLUGIN_INFO("plugin connected %d", m_time_connected);
}

void Connectee::OnDisconnected(const ConnectionNotification& notification) {
	m_time_disconnected = Base::Time::GetTimeMs();
	PLUGIN_INFO("plugin disconnected %d", m_time_disconnected);
}

void Connectee::OnRecvReady(const ConnectionNotification& notification) {
	SimplePlugin::Recv(notification.handle, m_recv_buffer, kRecvBufferSize, [&](void* buffer, unsigned int nbytes){
		m_bytes_received += nbytes;
	});
}
