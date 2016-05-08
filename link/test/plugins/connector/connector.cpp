/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "connector.h"
#include "link/plugin_log.h"

#include "base/core/time_utils.h"

Connector::Connector()
 : SimplePlugin(kUpdateDeltaMs)
 , m_target_watch(0)
 , m_target_connection(0)
 , m_time_connected(0)
 , m_time_disconnected(0)
 , m_bytes_sent(0){

}

bool Connector::OnStartup(const char* config, streamsize nbytes) {
	m_target_watch = CreateWatch("connectee", "*", "*");

	if(m_target_watch == 0) {
		return false;
	}

	PLUGIN_INFO("started: %dms", Base::Time::GetTimeMs());

	return true;
}

void Connector::OnShutdown() {
	CloseWatch(m_target_watch);
	m_target_watch = 0;

	if(0 == m_time_disconnected) { // todo; fix to get disconnected notifs before shutdown.
		PLUGIN_WARN("disconnection not notified");
		m_time_disconnected = Base::Time::GetTimeMs();
		PLUGIN_INFO("target disconnected: %dms", m_time_disconnected);
	}

	unsigned int duration_ms = m_time_disconnected - m_time_connected;
	unsigned int duration_sec = duration_ms / 1000;
	u64 kb_sent = m_bytes_sent / 1000;
	u64 mb_sent = kb_sent / 1000;

	PLUGIN_INFO("upstream: sent %luB in %dms", m_bytes_sent, duration_ms);
	PLUGIN_INFO("upstream: %luB/s. %luKB/s. %luMB/s",
			m_bytes_sent / duration_sec,
			kb_sent / duration_sec,
			mb_sent / duration_sec);
}

void Connector::OnWatchMatch(const WatchNotification& notification) {
	if(notification.handle == m_target_watch) {
		PLUGIN_INFO("watch matched.");
		m_target_connection = Connect(notification.plugin);
		if(0 == m_target_connection) {
			PLUGIN_ERROR("failed connecting to target plugin");
		}
	}
}

void Connector::OnConnected(const ConnectionNotification& notification) {
	if(m_target_connection == notification.handle) {
		m_time_connected = Base::Time::GetTimeMs();
		PLUGIN_INFO("connected to target: %dms", m_time_connected);
	}
}

void Connector::OnDisconnected(const ConnectionNotification& notification) {
	if(m_target_connection == notification.handle) {
		m_target_connection = 0;
		m_time_disconnected = Base::Time::GetTimeMs();
		PLUGIN_INFO("target disconnected: %dms", m_time_disconnected);
	}
}

void Connector::OnUpdate(unsigned int dt) {
	(void)dt;
	if(0 != m_target_connection) {
		const char sData[kMessageSize] = {};
		const unsigned int sDataSize = sizeof(sData);
		bool sent = 0 == Send(m_target_connection, sData, sDataSize);
		if(!sent) {
			PLUGIN_ERROR("failed sending data");
		} else {
			//PLUGIN_INFO("sent: %dms", Base::Time::GetTimeMs());
			m_bytes_sent += sDataSize;
		}
	}
}
