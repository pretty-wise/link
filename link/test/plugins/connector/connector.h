/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "plugin_interface.h"
#include "link/link.h"

class Connector : public SimplePlugin {
public:
	Connector();
	bool OnStartup(const char* config, streamsize nbytes);
	void OnShutdown();
	void OnWatchMatch(const WatchNotification& notification);
	void OnConnected(const ConnectionNotification& notification);
	void OnDisconnected(const ConnectionNotification& notification);
	void OnUpdate(unsigned int dt);

	enum CONSTANT {
		kUpdateDeltaMs = 10,
		kMessageSize = 1024 * 8
	};
private:
	WatchHandle m_target_watch;
	ConnectionHandle m_target_connection;

	unsigned int m_time_connected;
	unsigned int m_time_disconnected;
	u64 m_bytes_sent;
};
