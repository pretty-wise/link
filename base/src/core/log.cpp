/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/core/log.h"
#include "base/core/time_utils.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <thread>
#include <vector>
#include <algorithm>

namespace Base {
namespace Log {

static int gHandleGenerator = 0;

struct HookData {
  int handle;
  LogChannel channel;
  int filter;
  LogHook callback;
  void *context;
};

// todo(kstasik): better init/release
static std::vector<HookData> gLogHooks;

static int gLogLevelMask =
    kLogDebug | kLogInfo | kLogWarning | kLogError | kLogCritical;

void SetLevelFilter(int level_mask) { gLogLevelMask = level_mask; }

int GetLevelFilter() { return gLogLevelMask; }

const char *GetLevelString(int level) {
  switch(level) {
  case kLogDebug:
    return "DBG";
  case kLogInfo:
    return "INF";
  case kLogWarning:
    return "WAR";
  case kLogError:
    return "ERR";
  case kLogCritical:
    return "CRI";
  default:
    break;
  }
  return "???";
}

const char *SkipParentDirs(const char *path) {
  const char *result = path;
  while(result[0] == '.' && result[1] == '.' && result[2] == '/') {
    result = &result[3];
  }
  return result;
}

const char *GetFilename(const char *path) {
  const char *filename = strrchr(path, '/');
  return filename ? filename + 1 : path;
}

void Write(const char *file, int line, int level, const LogChannel &channel,
           const char *format, va_list arg_list) {
  const int max_line_length = Base::Log::kLogLineMaxLength;
  char logline[max_line_length];
  // NOTE: remove three bytes to ensure that there is space for the CRLF
  //			 and the null terminator.
  int avail = max_line_length - 3;
  int offset = 0;
  int n = 0;

  const char *level_string = GetLevelString(level);
  n = snprintf(&logline[offset], avail, "%d|%s|%s|", Base::Time::GetTimeMs(),
               level_string, channel.GetName());
  offset += n;
  avail -= n;

  // write the log message
  n = vsnprintf(&logline[offset], avail, format, arg_list);
  if(n >= 0 && n < avail) {
    offset += n;
    avail -= n;
  } else {
    offset = max_line_length - 3;
    avail = 0;
  }

  // early out if there is no message or buffer full.
  if(0 >= n || avail < 0) {
    return;
  }

  // write the log suffix
  file = GetFilename(file);
  n = snprintf(&logline[offset], avail, " file=%s line=%d", file, line);

  if(n >= 0 && n < avail) {
    offset += n;
    avail -= n;
  } else {
    offset = max_line_length - 3;
    avail = 0;
  }

  logline[offset++] = '\r';
  logline[offset++] = '\n';
  logline[offset] = 0;

  if(gLogHooks.empty()) {
    ConsoleOutput(level, logline, offset, nullptr);
  }

  for(auto it : gLogHooks) {
    if((it.channel == kAnyCategory || it.channel == channel) &&
       (it.filter & level) != 0) {
      it.callback(level, logline, offset, it.context);
    }
  }
}

void ConsoleOutput(int level, const char *logline, int length, void *context) {
  printf("%s", logline);
}

void Write(const char *file, int line, int level, const LogChannel &channel,
           const char *format, ...) {
  va_list args;
  va_start(args, format);
  Write(file, line, level, channel, format, args);
  va_end(args);
}

int Register(int filter, LogHook callback, void *context) {
  return Register(kAnyCategory, filter, callback, context);
}

int Register(const LogChannel &channel, int filter, LogHook callback,
             void *context) {
  int handle = ++gHandleGenerator;
  HookData data = {handle, channel, filter, callback, context};
  gLogHooks.push_back(data);
  return handle;
}

void Unregister(int handle) {
  gLogHooks.erase(std::remove_if(begin(gLogHooks), end(gLogHooks),
                                 [handle](const HookData &data) {
                                   return handle == data.handle;
                                 }),
                  end(gLogHooks));
}

} // namespace Log
} // namespace Base
