/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/io/base_file.h"
#include "monitor_plugin.h"

#include "base/core/assert.h"
#include "base/core/time_utils.h"
#include "common/json/json_writer.h"
#include "link/plugin_log.h"

#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/task_info.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace Link {
/*
template <> inline void JsonWriter::AppendValue(const ProcessPercent &info) {
  JsonWriter writer(m_destination);
  writer.Write("user", info.user);
  writer.Write("system", info.system);
  writer.Finalize();
}
*/

static const u64 kMicrosecondsPerSecond = 1000000;

bool Sample(u64 &user_micro, u64 &system_micro) {
  auto self = getpid();
  task_t task;

  kern_return_t error = task_for_pid(mach_task_self(), self, &task);
  if(error != KERN_SUCCESS) {
    return false;
  }

  mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;
  struct task_basic_info taskinfo;
  error = task_info(task, TASK_BASIC_INFO, (task_info_t)&taskinfo, &count);
  if(error != KERN_SUCCESS) {
    return false;
  }

  task_thread_times_info thread_info;
  count = TASK_THREAD_TIMES_INFO_COUNT;
  error = task_info(task, TASK_THREAD_TIMES_INFO, (task_info_t)&thread_info,
                    &count);
  if(error != KERN_SUCCESS) {
    return false;
  }

  user_micro = thread_info.user_time.seconds * kMicrosecondsPerSecond +
               thread_info.user_time.microseconds;
  system_micro = thread_info.system_time.seconds * kMicrosecondsPerSecond +
                 thread_info.system_time.microseconds;
  return true;
}

struct CPUStatsCmd::PIMPL {
  u64 m_last_user;
  u64 m_last_system;

  u64 m_last_user_diff;
  u64 m_last_system_diff;

  u64 m_last_timestamp;
  u64 m_last_timeslice;
};

CPUStatsCmd::CPUStatsCmd() { m_pimpl = new PIMPL; }
CPUStatsCmd::~CPUStatsCmd() { delete m_pimpl; }

void CPUStatsCmd::Sample() {
  u64 user, system;
  if(!Link::Sample(user, system))
    return;

  m_pimpl->m_last_user_diff = user - m_pimpl->m_last_user;
  m_pimpl->m_last_system_diff = system - m_pimpl->m_last_system;

  m_pimpl->m_last_user = user;
  m_pimpl->m_last_system = system;

  timeval time;
  gettimeofday(&time, NULL);

  u64 timestamp = (time.tv_sec * kMicrosecondsPerSecond) + time.tv_usec;
  m_pimpl->m_last_timeslice = timestamp - m_pimpl->m_last_timestamp;
  m_pimpl->m_last_timestamp = timestamp;
}

bool CPUStatsCmd::OnCommand(const std::string &query_string,
                            const std::string &post_data,
                            std::string *response_data) {
  JsonWriter writer(*response_data);
  writer.Write("platform", "mac");
  writer.Write("error", false);
  writer.Write("timestamp", m_pimpl->m_last_timestamp);
  writer.Write("timeslice", m_pimpl->m_last_timeslice);
  writer.Write("user", m_pimpl->m_last_user_diff);
  writer.Write("system", m_pimpl->m_last_system_diff);
  writer.Finalize();
  return true;
}

} // namespace Link
