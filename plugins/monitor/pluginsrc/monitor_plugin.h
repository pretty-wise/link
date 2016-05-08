/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include "plugin_interface.h"
#include "plugin/rest/rest_client.h"
#include "plugin/rest/rest_command.h"

namespace Link {

class CPUStatsCmd : public RestCommand {
public:
	CPUStatsCmd();
	virtual ~CPUStatsCmd();
	const char* Name() const { return "cpu-stats"; }

	void Sample();
	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data);
private:
	struct PIMPL;

	PIMPL* m_pimpl;
};

class MemStatsCmd : public RestCommand {
public:
	MemStatsCmd(){}
	virtual ~MemStatsCmd(){}
	const char* Name() const { return "mem-stats"; }
	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data);
private:
	struct PIMPL;
	PIMPL* m_pimpl;
};

class MonitorPlugin : public SimplePlugin, public RestClient {
public:
	MonitorPlugin();
	~MonitorPlugin();
	bool OnStartup(const char* config, streamsize nbytes);
	void OnShutdown();

	void OnNotification(const Notification& notif);
	void OnRecvReady(const ConnectionNotification& notif);
	void OnUpdate(unsigned int dt);
 
	enum CONSTANTS {
		kUpdateDeltaMs = 10,
		kRecvBufferSize = 1024
	};
private:
	void* m_recv_buffer;

	CPUStatsCmd m_cpu_stats_cmd;
	MemStatsCmd m_mem_stats_cmd;
};

} // namespace Link
