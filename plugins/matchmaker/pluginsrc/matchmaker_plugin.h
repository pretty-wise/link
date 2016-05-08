/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "plugin_interface.h"
#include "plugin/matchmaker/matchmaker_defs.h"
#include "plugin/rest/rest_client.h"

namespace Link {
namespace Matchmaker {

class MatchmakerPlugin : public SimplePlugin, public RestClient {
public:
	MatchmakerPlugin();
	~MatchmakerPlugin();
	bool OnStartup(const char* config, streamsize nbytes) override;
	void OnShutdown() override;

	void OnNotification(const Notification& notif) override;
	void OnRecvReady(const ConnectionNotification& notif) override;
	void OnUpdate(unsigned int dt) override;
	void OnPluginConnected(const ConnectionNotification& notif) override;
	void OnConnected(const ConnectionNotification& notif) override;
	void OnDisconnected(const ConnectionNotification& notif) override;
	
	enum CONSTANTS {
		kUpdateDeltaMs = 10,
		kRecvBufferSize = 1024
	};
	
private:
	void ParseDataReceived(void* buffer, unsigned int nbytes, ConnectionHandle connection, PluginHandle plugin);
	void* m_recv_buffer;
};

} // namespace Matchmaker 
} // namespace Link

