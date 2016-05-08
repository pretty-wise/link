/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "monitor_plugin.h"
#include "base/io/base_file.h"

#include "common/json/json_writer.h"
#include "link/plugin_log.h"
#include "base/core/time_utils.h"
#include "base/core/assert.h"

#include <unistd.h>

namespace Link {

static const streamsize kProcBufferSize = 1024;

struct CPUSample {
  u32 timestamp; // sample time.
  u32 user;      // normal processes executing in user mode.
  u32 nice;      // niced processes executing in user mode.
  u32 system;    // processes executing in krenel mode.
  u32 idle;      // twiddling thumbs.
  u32 iowait;    // waiting for I/O to complete.
  u32 irq;       // servicing interrupts.
  u32 softirq;   // servicing softirqs.

  void InitMax() {
    timestamp = (u32)-1;
    user = (u32)-1;
    nice = (u32)-1;
    system = (u32)-1;
    idle = (u32)-1;
    iowait = (u32)-1;
    irq = (u32)-1;
    softirq = (u32)-1;
  }

  void GetMax(const CPUSample &a) {
    if(a.timestamp > timestamp)
      timestamp = a.timestamp;
    if(a.user > user)
      user = a.user;
    if(a.nice > nice)
      nice = a.nice;
    if(a.system > system)
      system = a.system;
    if(a.idle > idle)
      idle = a.idle;
    if(a.iowait > iowait)
      iowait = a.iowait;
    if(a.irq > irq)
      irq = a.irq;
    if(a.softirq > softirq)
      softirq = a.softirq;
  }

  void GetMin(const CPUSample &a) {
    if(a.timestamp < timestamp)
      timestamp = a.timestamp;
    if(a.user < user)
      user = a.user;
    if(a.nice < nice)
      nice = a.nice;
    if(a.system < system)
      system = a.system;
    if(a.idle < idle)
      idle = a.idle;
    if(a.iowait < iowait)
      iowait = a.iowait;
    if(a.irq < irq)
      irq = a.irq;
    if(a.softirq < softirq)
      softirq = a.softirq;
  }

  CPUSample &operator+=(const CPUSample &rhs) {
    timestamp += rhs.timestamp;
    user += rhs.user;
    nice += rhs.nice;
    system += rhs.system;
    idle += rhs.idle;
    iowait += rhs.iowait;
    irq += rhs.irq;
    softirq += rhs.softirq;
    return *this;
  }
};

struct ProcessSample {
  u32 timestamp;
  u32 user;
  u32 system;
};

float ToPercent(u32 stat, u32 time, u32 hz) {
  float dt_sec = static_cast<float>(time) * 0.001f;
  float denom = dt_sec * static_cast<float>(hz);
  if(denom == 0.f) {
    return 0.f;
  }
  return static_cast<float>(stat) / denom;
}

struct ProcessPercent {
  float user;
  float system;

  void Calculate(const ProcessSample &delta, u32 hz) {
    user = ToPercent(delta.user, delta.timestamp, hz);
    system = ToPercent(delta.system, delta.timestamp, hz);
  }
};

template <> inline void JsonWriter::AppendValue(const ProcessPercent &info) {
  JsonWriter writer(m_destination);
  writer.Write("user", info.user);
  writer.Write("system", info.system);
  writer.Finalize();
}

struct ProcessHistory {
  u32 num_samples;
  u32 current_sample;
  ProcessSample *data;

  ProcessHistory() : num_samples(0), current_sample(0), data(nullptr) {}

  ~ProcessHistory() { delete[] data; }

  void Resize(u32 _num_samples) {
    BASE_ASSERT(_num_samples > 0);
    num_samples = _num_samples;
    current_sample = 0;
    delete[] data;
    data = new ProcessSample[num_samples];
  }

  ProcessSample *GetNextSample() {
    BASE_ASSERT(current_sample < num_samples, "sample out of range");
    ProcessSample *res = &data[current_sample];
    current_sample = (++current_sample) % num_samples;
    return res;
  }

  void LastSampleStats(u32 tick_per_sec, ProcessPercent *out) {
    ProcessSample diff;
    Diff(data[(current_sample - 1) % num_samples],
         data[(current_sample - 2) % num_samples], &diff);
    out->Calculate(diff, tick_per_sec);
  }

  void Diff(const ProcessSample &a, const ProcessSample &b,
            ProcessSample *res) {
    res->user = a.user - b.user;
    res->system = a.system - b.system;
  }
};

struct CPUPercent {
  float user;
  float nice;
  float system;
  float idle;
  float iowait;
  float irq;
  float softirq;

  void InitMax() {
    user = 1.f;
    nice = 1.f;
    system = 1.f;
    idle = 1.f;
    iowait = 1.f;
    irq = 1.f;
    softirq = 1.f;
  }

  void GetMin(const CPUPercent &d) {
    if(d.user < user)
      user = d.user;
    if(d.nice < nice)
      nice = d.nice;
    if(d.system < system)
      system = d.system;
    if(d.idle < idle)
      idle = d.idle;
    if(d.iowait < iowait)
      iowait = d.iowait;
    if(d.irq < irq)
      irq = d.irq;
    if(d.softirq < softirq)
      softirq = d.softirq;
  }

  void GetMax(const CPUPercent &d) {
    if(d.user > user)
      user = d.user;
    if(d.nice > nice)
      nice = d.nice;
    if(d.system > system)
      system = d.system;
    if(d.idle > idle)
      idle = d.idle;
    if(d.iowait > iowait)
      iowait = d.iowait;
    if(d.irq > irq)
      irq = d.irq;
    if(d.softirq > softirq)
      softirq = d.softirq;
  }

  void Calculate(const CPUSample &delta, u32 hz) {
    user = ToPercent(delta.user, delta.timestamp, hz);
    nice = ToPercent(delta.nice, delta.timestamp, hz);
    system = ToPercent(delta.system, delta.timestamp, hz);
    idle = ToPercent(delta.idle, delta.timestamp, hz);
    iowait = ToPercent(delta.iowait, delta.timestamp, hz);
    irq = ToPercent(delta.irq, delta.timestamp, hz);
    softirq = ToPercent(delta.softirq, delta.timestamp, hz);
  }
};

template <> inline void JsonWriter::AppendValue(const CPUPercent &info) {
  JsonWriter writer(m_destination);
  writer.Write("user", info.user);
  writer.Write("nice", info.nice);
  writer.Write("system", info.system);
  writer.Write("idle", info.idle);
  writer.Write("iowait", info.iowait);
  writer.Write("irq", info.irq);
  writer.Write("softirq", info.softirq);
  writer.Finalize();
}

struct CPUHistory {
  u32 num_samples;
  u32 current_sample;
  struct CPUSample *data;
  struct CPUPercent min, max, avg;

  CPUHistory() : num_samples(0), current_sample(0), data(0) {}
  ~CPUHistory() { delete[] data; }

  void Resize(u32 _num_samples) {
    BASE_ASSERT(_num_samples > 0);
    num_samples = _num_samples;
    current_sample = 0;
    delete[] data;
    data = new CPUSample[num_samples];
  }

  CPUSample *GetNextSample() {
    BASE_ASSERT(current_sample < num_samples, "sample out of range");
    CPUSample *res = &data[current_sample];
    current_sample = (++current_sample) % num_samples;
    return res;
  }

  void LastSampleStats(u32 tick_per_sec, CPUPercent *out) {
    CPUSample diff;
    Diff(data[(current_sample - 1) % num_samples],
         data[(current_sample - 2) % num_samples], &diff);
    out->Calculate(diff, tick_per_sec);
  }

  void Calculate(u32 tick_per_sec) {
    CPUSample sum = {}, diff;
    min.InitMax();
    for(u32 i = 0; i < num_samples; ++i) {
      Diff(data[(i - 1) % num_samples], data[(i - 2) % num_samples], &diff);
      sum += diff;
      CPUPercent tmp;
      tmp.Calculate(diff, tick_per_sec);
      min.GetMin(tmp);
      max.GetMax(tmp);
    }
    avg.Calculate(sum, tick_per_sec);
  }

  void Diff(const CPUSample &a, const CPUSample &b, CPUSample *res) {
    res->timestamp = a.timestamp - b.timestamp;
    res->user = a.user - b.user;
    res->nice = a.nice - b.nice;
    res->system = a.system - b.system;
    res->idle = a.idle - b.idle;
    res->iowait = a.iowait - b.iowait;
    res->irq = a.irq - b.irq;
    res->softirq = a.softirq - b.softirq;
  }
};

template <> inline void JsonWriter::AppendValue(const CPUSample &info) {
  JsonWriter writer(m_destination);
  writer.Write("timestamp", info.timestamp);
  writer.Write("user", info.user);
  writer.Write("nice", info.nice);
  writer.Write("system", info.system);
  writer.Write("idle", info.idle);
  writer.Write("iowait", info.iowait);
  writer.Write("irq", info.irq);
  writer.Write("softirq", info.softirq);
  writer.Finalize();
}

char *GoToNextValue(char *cur) {
  // skip current value.
  while(*cur != ' ' && *cur != '\t') {
    ++cur;
  }
  // skip whitespaces.
  while(*cur == ' ' || *cur == '\t') {
    ++cur;
  }
  return cur;
}

char *GoToNextLine(char *cur) {
  while(*cur != '\n' && *cur != 0) {
    ++cur;
  }
  if(*cur == '\n') {
    ++cur;
  }
  return cur;
}

bool ReadCPUStatus(char *&pointer, struct CPUSample *stats, u32 timestamp) {
  // field definition: http://man7.org/linux/man-pages/man5/proc.5.html

  if(pointer[0] != 'c' || pointer[1] != 'p' || pointer[2] != 'u') {
    return false;
  }

  stats->timestamp = timestamp;

  pointer = GoToNextValue(pointer);

  Base::String::FromString(pointer, stats->user);
  pointer = GoToNextValue(pointer);
  Base::String::FromString(pointer, stats->nice);
  pointer = GoToNextValue(pointer);
  Base::String::FromString(pointer, stats->system);
  pointer = GoToNextValue(pointer);
  Base::String::FromString(pointer, stats->idle);
  pointer = GoToNextValue(pointer);
  Base::String::FromString(pointer, stats->iowait);
  pointer = GoToNextValue(pointer);
  Base::String::FromString(pointer, stats->irq);
  pointer = GoToNextValue(pointer);
  Base::String::FromString(pointer, stats->softirq);
  pointer = GoToNextLine(pointer);
  return true;
}

bool ReadCPUStats(struct CPUHistory *total, struct CPUHistory **cpu,
                  int num_cpu) {
  Base::FileHandle file = Base::Open("/proc/stat", Base::OM_Read);
  if(file == -1) {
    PLUGIN_ERROR("failed opeining proc stat");
    return false;
  }

  char buffer[kProcBufferSize];
  size_t nbytes = Base::Read(file, buffer, kProcBufferSize);
  if(nbytes == static_cast<size_t>(-1)) {
    PLUGIN_ERROR("problem reading proc stat");
    return false;
  }

  u32 timestamp = Base::Time::GetTimeMs();

  char *pointer = buffer;
  if(!ReadCPUStatus(pointer, total->GetNextSample(), timestamp)) {
    PLUGIN_ERROR("problem reading cpu stats");
    Base::Close(file);
    return false;
  }

  for(int i = 0; i < num_cpu; ++i) {
    if(!pointer ||
       !ReadCPUStatus(pointer, cpu[i]->GetNextSample(), timestamp)) {
      PLUGIN_ERROR("problem reading %d cpu stats", i);
      Base::Close(file);
      return false;
    }
  }

  Base::Close(file);
  return true;
}

bool ReadProcessStats(struct ProcessHistory *hist) {
  Base::FileHandle file = Base::Open("/proc/self/stat", Base::OM_Read);

  char buffer[kProcBufferSize];
  size_t nbytes = Base::Read(file, buffer, kProcBufferSize);
  if(nbytes == static_cast<size_t>(-1)) {
    PLUGIN_ERROR("problem reading proc stat");
    return false;
  }

  char *pointer = buffer;

  for(int i = 0; i < 13; ++i) {
    pointer = GoToNextValue(pointer);
  }

  ProcessSample *sample = hist->GetNextSample();

  Base::String::FromString(pointer, sample->user);
  pointer = GoToNextValue(pointer);
  Base::String::FromString(pointer, sample->system);

  Base::Close(file);
  return true;
}

struct CPUStatsCmd::PIMPL {
  u32 m_num_cpu;            // number of processors
  u32 m_ticks_per_sec;      // clock speed.
  ProcessHistory m_process; // process stat data.
  CPUHistory m_total;       // all cpu stat data.
  CPUHistory *m_cpu;        // stat data per cpu.
  u32 m_last_read_time;     // last time stats were read.
  bool m_error;             // true if there was a processing error

  PIMPL(u32 sample_count) : m_error(false) {
    m_num_cpu = sysconf(_SC_NPROCESSORS_ONLN);
    m_ticks_per_sec = sysconf(_SC_CLK_TCK);
    m_cpu = new CPUHistory[m_num_cpu];

    m_total.Resize(sample_count);
    for(u32 i = 0; i < m_num_cpu; ++i) {
      m_cpu[i].Resize(sample_count);
    }
    m_process.Resize(sample_count);
    // init stats.
    ReadStats();
  }
  ~PIMPL() { delete[] m_cpu; }

  void ReadStats() {
    ReadCPUStats(&m_total, &m_cpu, m_num_cpu);
    ReadProcessStats(&m_process);
    m_last_read_time = Base::Time::GetTimeMs();
  }

  void Recalculate() {
    m_total.Calculate(m_ticks_per_sec);
    for(u32 i = 0; i < m_num_cpu; ++i) {
      m_cpu[i].Calculate(m_ticks_per_sec);
    }
  }
};

CPUStatsCmd::CPUStatsCmd() {
  u32 sample_count = 10;
  m_pimpl = new PIMPL(sample_count);
}

CPUStatsCmd::~CPUStatsCmd() { delete m_pimpl; }

void CPUStatsCmd::Sample() {}

bool CPUStatsCmd::OnCommand(const std::string &query_string,
                            const std::string &post_data,
                            std::string *response_data) {

  u32 prev_read_time = m_pimpl->m_last_read_time;
  m_pimpl->ReadStats();
  m_pimpl->Recalculate();

  CPUPercent total;
  m_pimpl->m_total.LastSampleStats(m_pimpl->m_ticks_per_sec, &total);

  ProcessPercent process;
  m_pimpl->m_process.LastSampleStats(m_pimpl->m_ticks_per_sec, &process);

  JsonWriter writer(*response_data);
  writer.Write("platform", "linux");
  writer.Write("error", m_pimpl->m_error);
  writer.Write("cpu_num", m_pimpl->m_num_cpu);
  writer.Write("ticks_per_sec", m_pimpl->m_ticks_per_sec);
  writer.Write("timestamp", m_pimpl->m_last_read_time);
  writer.Write("timeslice", m_pimpl->m_last_read_time - prev_read_time);
  writer.Write("total", total);
  writer.Write("process", process);
  writer.Finalize();
  return true;
}

} // namespace Link
