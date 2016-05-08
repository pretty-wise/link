/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "plugin_interface.h"
#include "link/link.h"

class Connectee : public SimplePlugin {
public:
	Connectee();
	bool OnStartup(const char* config, streamsize nbytes);
	void OnShutdown();
	void OnConnected(const ConnectionNotification& notification);
	void OnRecvReady(const ConnectionNotification& notification);
	void OnDisconnected(const ConnectionNotification& notification);

	enum CONSTANT {
		kUpdateDeltaMs = 10,
		kRecvBufferSize = 1024 * 100
	};
private:
	void* m_recv_buffer;
	ConnectionHandle m_target_connection;

	u32 m_time_connected;
	u32 m_time_disconnected;
	u64 m_bytes_received;
};
