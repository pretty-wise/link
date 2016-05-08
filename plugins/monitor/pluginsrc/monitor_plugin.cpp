/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "monitor_plugin.h"

#include "link/plugin_log.h"
#include "base/core/str.h"
#include "base/math/crc.h"
#include "base/core/assert.h"

#include "json_utils.h"
#include "common/json/json_reader.h"
#include "common/json/json_writer.h"

const char *SimplePlugin::Name = "monitor";
const char *SimplePlugin::Version = "0.1";

SimplePlugin *SimplePlugin::CreatePlugin() { return new Link::MonitorPlugin(); }

void SimplePlugin::DestroyPlugin(SimplePlugin *plugin) { delete plugin; }

namespace Link {

bool MemStatsCmd::OnCommand(const std::string &query_string,
                            const std::string &post_data,
                            std::string *response_data) {
  *response_data = "{ not implemented }";
  return true;
}

MonitorPlugin::MonitorPlugin() : SimplePlugin(kUpdateDeltaMs) {
  m_recv_buffer = malloc(kRecvBufferSize);
}

MonitorPlugin::~MonitorPlugin() { free(m_recv_buffer); }

bool MonitorPlugin::OnStartup(const char *config, streamsize nbytes) {
  if(!RegisterCommand(&m_cpu_stats_cmd)) {
    return false;
  }

  if(!RegisterCommand(&m_mem_stats_cmd)) {
    return false;
  }

  return true;
}

void MonitorPlugin::OnShutdown() {
  UnregisterCommand(&m_cpu_stats_cmd);
  UnregisterCommand(&m_mem_stats_cmd);
}

void MonitorPlugin::OnUpdate(unsigned int dt) { m_cpu_stats_cmd.Sample(); }

void MonitorPlugin::OnRecvReady(const ConnectionNotification &notif) {
  SimplePlugin::Recv(notif.handle, m_recv_buffer, kRecvBufferSize,
                     [&](void *buffer, unsigned int nbytes) {
                       // ParseDataReceived(buffer, nbytes, notif.handle,
                       // notif.endpoint);
                     });
}

void MonitorPlugin::OnNotification(const Notification &notif) {
  RestClient::ProcessNotification(notif);
}

} // namespace Link
