/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/core/types.h"
#include "base/core/log.h"
#include "link/plugin_log.h"

#include <stdarg.h>

const Base::LogChannel kDefaultLinkLog("link");

#define LINK_DEBUG(...)                                                        \
  Base::Log::Write(__FILE__, __LINE__, Base::Log::kLogDebug, kDefaultLinkLog, __VA_ARGS__)
#define LINK_INFO(...)                                                         \
  Base::Log::Write(__FILE__, __LINE__, Base::Log::kLogInfo, kDefaultLinkLog, __VA_ARGS__)
#define LINK_WARN(...)                                                         \
  Base::Log::Write(__FILE__, __LINE__, Base::Log::kLogWarning, kDefaultLinkLog,           \
                   __VA_ARGS__)
#define LINK_ERROR(...)                                                        \
  Base::Log::Write(__FILE__, __LINE__, Base::Log::kLogError, kDefaultLinkLog, __VA_ARGS__)
#define LINK_CRITICAL(...)                                                     \
  Base::Log::Write(__FILE__, __LINE__, Base::Log::kLogCritical, kDefaultLinkLog,          \
                   __VA_ARGS__)
#define LINK_LOG(channel, level, ...)                                          \
  Base::Log::Write(__FILE__, __LINE__, level, channel, __VA_ARGS__)

namespace Link {
namespace Log {

/// Plugin specific log write function. Do not use directly.
/// Reserved for use by the plugin object instances.

void PluginWrite(const char *file, int line, int category,
                 const char *plugin_name, const char *plugin_version,
                 const char *format, va_list arg_list);

} // Log
} // namespace Link
