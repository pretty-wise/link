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

static int log_category_mask = kLogInfo | kLogWarning | kLogError | kLogCritical;

void SetCategoryFilter(int category_mask) {
	log_category_mask = category_mask;
}

int GetCategoryFilter() {
	return log_category_mask;
}

const char* GetCategoryString(int category) {
	switch(category) {
		case kLogDebug:		return 	"DBG";
		case kLogInfo:		 return "INF";
		case kLogWarning:	return 	"WAR";
		case kLogError:		return 	"ERR";
		case kLogCritical: return "CRI";
		default: break;
	}
	return "UNKNOWN	";
}

const char* SkipParentDirs(const char* path)
{
	const char* result = path;
	while(result[0] == '.'
		&& result[1] == '.'
		&& result[2] == '/') {
		result = &result[3];
	}
	return result;
}

void WriteImpl(const char* file,
											int line,
											int category,
											const char* plugin_name,
											const char* plugin_version,
											const char* format,
											va_list arg_list) {

	if(0 == (log_category_mask&category)) {
		return;
	}

	const int max_line_length = Base::Log::kLogLineMaxLength;
	char logline[max_line_length];
	// NOTE: remove three bytes to ensure that there is space for the CRLF
	//			 and the null terminator.
	int avail = max_line_length - 3;
	int offset = 0;
	int n = 0;

	const char* category_string = GetCategoryString(category);

	if(plugin_name && plugin_version) {
		// write the plugin name and version if specified
		n = snprintf(&logline[offset], avail, "%d|%s|plugin=%s(%s)|msg=", Base::Time::GetTimeMs(), category_string, plugin_name, plugin_version);
		offset += n;
		avail -= n;
	} else {
		// write the category
		n = snprintf(&logline[offset], avail, "%d|%s|plugin=link_core(%d)|msg=", Base::Time::GetTimeMs(), 
																																						category_string, 
																																						std::this_thread::get_id());
		offset += n;
		avail -= n;
	}

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
	n = snprintf(&logline[offset], avail, " file=%s line=%d", SkipParentDirs(file), line);
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

	//for(int i = 0; i < num_write_hooks; ++i) {
	//	write_hook_data[i].hook(category, logline, offset, write_hook_data[i].context);
	//}

	// log hooks here.

	printf("%s", logline);
}

void Write(const char* file,
					 int line,
					 int category,
					 const char* format,
					 ...) {
	va_list args;
	va_start(args, format);
	WriteImpl(file, line, category, 0, 0, format, args);
	va_end(args);
}

void PluginWrite(const char* file,
								 int line,
								 int category,
								 const char* plugin_name,
								 const char* plugin_version,
								 const char* format,
								 va_list arg_list) {
	WriteImpl(file, line, category, plugin_name, plugin_version, format, arg_list);
}

} // namespace Log

} // namespace Link
