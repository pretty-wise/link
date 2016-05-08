/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

enum LogCategory {
	kLogDebug		= (0x1<<0), /*!< Debug category */
	kLogInfo		 = (0x1<<1), /*!< Informational category */
	kLogWarning	= (0x1<<2), /*!< Warning category */
	kLogError		= (0x1<<3), /*!< Recoverable error category */
	kLogCritical = (0x1<<4), /*!< Non-recoverable error category */
};

#define PLUGIN_DEBUG(...) \
	LogWritePlugin(__FILE__, __LINE__, kLogDebug, __VA_ARGS__)

#define PLUGIN_INFO(...) \
	LogWritePlugin(__FILE__, __LINE__, kLogInfo, __VA_ARGS__)

#define PLUGIN_WARN(...) \
	LogWritePlugin(__FILE__, __LINE__, kLogWarning, __VA_ARGS__)

#define PLUGIN_ERROR(...) \
	LogWritePlugin(__FILE__, __LINE__, kLogError, __VA_ARGS__)

#define PLUGIN_CRITICAL(...) \
	LogWritePlugin(__FILE__, __LINE__, kLogCritical, __VA_ARGS__)


#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

void LogWritePlugin(const char* file,
								 int line,
								 int category,
								 const char* format,
								 ...);

#ifdef __cplusplus
}; /* extern "C" */
#endif /*__cplusplus*/
