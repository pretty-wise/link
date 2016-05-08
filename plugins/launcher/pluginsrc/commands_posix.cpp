/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "launcher.h"
#include "link/plugin_log.h"
#include "common/json/json_reader.h"
#include "common/json/json_writer.h"

#include <spawn.h>
#include <errno.h>
#include <signal.h>

extern char** environ;

namespace Link {

bool RunCmd::OnCommand(const std::string& query,
												const std::string& post_data,
												std::string* response_data) {
	PLUGIN_INFO("run processes query %s post\n %s", query.c_str(), post_data.c_str());

	Link::JsonReader reader;
	if(!reader.Parse(post_data.c_str(), post_data.length())) {
		*response_data = "cannot parse";
		return true;
	}

	std::string config;
	if(!reader.Read("config", &config)) {
		*response_data = "no config";
		return true;
	}

	if(m_pids->size() > 32) {
		*response_data = "limit hit";
		return true;
	}

	pid_t pid;
	std::string conf_arg = "--config=";
	conf_arg += config;
	char* const executable = "bin/link_server";
	char * const args[] = {
		"link_server",
		const_cast<char* const>(conf_arg.c_str()),
		0
	};
	int res = posix_spawn(&pid, executable, 0, 0, args, environ);
	if(res != 0) {
		PLUGIN_ERROR("process spawn failed with %d", res);
		*response_data = "failed";
		return true;
	}

	m_pids->push_back(pid);

	*response_data = "running";
	return true;
}

bool KillCmd::OnCommand(const std::string& query,
												const std::string& post_data,
												std::string* response_data) {
	PLUGIN_INFO("kill processes");

	Link::JsonReader reader;
	if(!reader.Parse(post_data.c_str(), post_data.length())) {
		*response_data = "cannot parse";
		return true;
	}

	pid_t pid = reader.Read("pid", 0);
	if(pid == 0) {
		*response_data = "cannot read pid";
		return true;
	}

	for(PidList::iterator it = m_pids->begin(); it != m_pids->end(); ++it) {
		if(*it == pid) {
			m_pids->erase(it);
			int res = kill(pid, SIGTERM);
			PLUGIN_INFO("kill resulted in %d", res);
			*response_data = "killed it";
			return true;
		}
	}

	*response_data = "process not found";
	return true;
}

bool ListProcessesCmd::OnCommand(const std::string& query,
												const std::string& post_data,
												std::string* response_data) {
	PLUGIN_INFO("list processes");

	Link::JsonWriter writer(*response_data);

	writer.Write("pids", *m_pids);
	return true;
}



} // namespace Link
