/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/core/log_file.h"
#include "base/io/base_file.h"
#include "base/process/process.h"

#include <string.h>
#include <vector>
#include <algorithm>

namespace Base {
namespace Log {

struct LogFileData {
  struct ChannelFilter {
    int handle;         // channel log hook handle.
    LogChannel channel; // channel to log.
    int filter;         // filter to apply on channel.
  };
  Base::FileHandle file;
  std::vector<ChannelFilter> channels;
};

size_t Flush(Base::FileHandle file, const void *data, int nbytes) {
  size_t wrote = Base::Write(file, data, nbytes);
  return wrote;
}
void FileLogHook(int level, const char *log_line, int length, void *context) {
  LogFileData *data = static_cast<LogFileData *>(context);
  // todo(kstasik): AutoLock(data->lock);
  Flush(data->file, static_cast<const void *>(log_line), length);
}

LogFile CreateLogFile(const char *path) {
  Base::FileHandle file = Base::Create(path);
  if(file == kInvalidHandle) {
    return nullptr;
  }
  LogFileData *data = new LogFileData();
  data->file = file;
  return data;
}

LogFile CreateLogFileUnique(const char *directory, const char *basename) {
  pid_t pid = Base::Process::getpid();

  const int kMaxPath = 512;
  char fullpath[kMaxPath];
  int avail = 512 - 1;
  int offset = 0;
  int n = 0;
  if(directory && strlen(directory) > 0) {
    n = snprintf(&fullpath[offset], avail, "%s/", directory);
    offset += n;
    avail -= n;
  }
  n = snprintf(&fullpath[offset], avail, "%s_%d.log", basename, pid);
  offset += n;
  avail -= n;
  return CreateLogFile(fullpath);
}

void DestroyLogFile(LogFile sink) {
  Close(sink->file);
  delete sink;
}

bool AddFilter(LogFile sink, const LogChannel &channel, int filter) {
  void *context = static_cast<void *>(sink);

  // check if the log hook has already been registered. for this channel.
  auto it =
      std::find_if(begin(sink->channels), end(sink->channels),
                   [channel, filter](const LogFileData::ChannelFilter &data) {
                     return data.channel == channel;
                   });
  if(it != sink->channels.end()) {
    if((*it).filter == filter) {
      // do nothing. channel with the same filter already registered
      return true;
    }
    // if log hook already registered and want to apply different filter
    // we have to unregister the old one and re-register new filter.
    Log::Unregister((*it).handle);
    int handle = Log::Register(channel, filter, FileLogHook, context);
    LogFileData::ChannelFilter data = {handle, channel, filter};
    sink->channels.push_back(data);
    return true;
  }
  // channel not yet registered
  int handle = Log::Register(channel, filter, FileLogHook, context);
  LogFileData::ChannelFilter data = {handle, channel, filter};
  sink->channels.push_back(data);
  return true;
}

void RemoveFilter(LogFile sink, const LogChannel &channel) {
  auto it = std::find_if(begin(sink->channels), end(sink->channels),
                         [channel](const LogFileData::ChannelFilter &data) {
                           return data.channel == channel;
                         });
  if(it != sink->channels.end()) {
    Log::Unregister((*it).handle);
    sink->channels.erase(it);
  }
}

} // namespace Log
} // namespace Base
