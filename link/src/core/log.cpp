/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "log.h"
#include "base/core/time_utils.h"
#include "base/core/log.h"
#include <stdio.h>
#include <thread>
#include <functional>

namespace Link {
namespace Log {

void ConsoleOutput(int category, const char *line, void *udata) {
  printf("%s", line);
}

const char *GetCategoryString(int category) {
  switch(category) {
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
  return "UNKNOWN	";
}

const char *SkipParentDirs(const char *path) {
  const char *result = path;
  while(result[0] == '.' && result[1] == '.' && result[2] == '/') {
    result = &result[3];
  }
  return result;
}

void PluginWrite(const char *file, int line, int category,
                 const char *plugin_name, const char *plugin_version,
                 const char *format, va_list arg_list) {
  const int max_line_length = Base::Log::kLogLineMaxLength;
  char logline[max_line_length];
  // NOTE: remove three bytes to ensure that there is space for the CRLF
  //			 and the null terminator.
  int avail = max_line_length - 3;

  if(plugin_name && plugin_version) {
    // write the plugin name and version if specified
    snprintf(logline, avail, "plugin=%s(%s)|%s", plugin_name, plugin_version,
             format);
  } else {
    // write the category
    snprintf(logline, avail, "plugin=link_core|%s", format);
  }
  Base::LogChannel channel = kDefaultLinkLog;
  Base::Log::Write(file, line, category, channel, logline, arg_list);
}

} // namespace Log
} // namespace Link
