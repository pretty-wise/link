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
