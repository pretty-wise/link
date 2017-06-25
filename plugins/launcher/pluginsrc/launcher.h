/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include "plugin_interface.h"
#include "plugin/rest/rest_client.h"
#include "plugin/rest/rest_command.h"

#include <map>
#include <vector>
#include <unistd.h>

namespace Link {

typedef std::vector<pid_t> PidList;

class RunCmd : public RestCommand {
public:
  RunCmd(PidList *pids) : m_pids(pids) {}
  const char *Name() const { return "run"; }
  virtual bool OnCommand(const std::string &query_string,
                         const std::string &post_data,
                         std::string *response_data);

private:
  PidList *m_pids;
};

class KillCmd : public RestCommand {
public:
  KillCmd(PidList *pids) : m_pids(pids) {}
  const char *Name() const { return "kill"; }
  virtual bool OnCommand(const std::string &query_string,
                         const std::string &post_data,
                         std::string *response_data);

private:
  PidList *m_pids;
};

class ListProcessesCmd : public RestCommand {
public:
  ListProcessesCmd(PidList *pids) : m_pids(pids) {}
  const char *Name() const { return "process-list"; }
  virtual bool OnCommand(const std::string &query_string,
                         const std::string &post_data,
                         std::string *response_data);

private:
  PidList *m_pids;
};

class Launcher : public SimplePlugin, public RestClient {
public:
  Launcher();
  ~Launcher();
  bool OnStartup(const char *config, streamsize nbytes);
  void OnShutdown();

  void OnNotification(const Notification &notif);
  void OnRecvReady(const ConnectionNotification &notif);
  void OnUpdate(unsigned int dt);

  enum CONSTANTS { kUpdateDeltaMs = 10, kRecvBufferSize = 1024 };

private:
  void *m_recv_buffer;
  PidList m_pids;

  ListProcessesCmd m_list;
  RunCmd m_run;
  KillCmd m_kill;
};

} // namespace Link
