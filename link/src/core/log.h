/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/core/types.h"

#include "link/plugin_log.h"

#include <stdarg.h>

#define LINK_DEBUG(...) \
	Link::Log::Write(__FILE__, __LINE__, kLogDebug, __VA_ARGS__)
#define LINK_INFO(...) \
	Link::Log::Write(__FILE__, __LINE__, kLogInfo, __VA_ARGS__)
#define LINK_WARN(...) \
	Link::Log::Write(__FILE__, __LINE__, kLogWarning, __VA_ARGS__)
#define LINK_ERROR(...) \
	Link::Log::Write(__FILE__, __LINE__, kLogError, __VA_ARGS__)
#define LINK_CRITICAL(...) \
	Link::Log::Write(__FILE__, __LINE__, kLogCritical, __VA_ARGS__)

namespace Link {
namespace Log {

void SetCategoryFilter(int category_mask);
int GetCategoryFilter();

void Write(const char* file,
					 int line,
					 int category,
					 const char* format,
					 ...);

/// Plugin specific log write function. Do not use directly.
/// Reserved for use by the dcn_plugin object instances.

void PluginWrite(const char* file,
								 int line,
								 int category,
								 const char* plugin_name,
								 const char* plugin_version,
								 const char* format,
								 va_list arg_list);

} // Log
} // namespace Link
