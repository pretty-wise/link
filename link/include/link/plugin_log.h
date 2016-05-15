/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/log.h"

#define PLUGIN_DEBUG(...)                                                      \
  LogWritePlugin(__FILE__, __LINE__, Base::Log::LogLevel::kLogDebug,           \
                 __VA_ARGS__)

#define PLUGIN_INFO(...)                                                       \
  LogWritePlugin(__FILE__, __LINE__, Base::Log::LogLevel::kLogInfo, __VA_ARGS__)

#define PLUGIN_WARN(...)                                                       \
  LogWritePlugin(__FILE__, __LINE__, Base::Log::LogLevel::kLogWarning,         \
                 __VA_ARGS__)

#define PLUGIN_ERROR(...)                                                      \
  LogWritePlugin(__FILE__, __LINE__, Base::Log::LogLevel::kLogError,           \
                 __VA_ARGS__)

#define PLUGIN_CRITICAL(...)                                                   \
  LogWritePlugin(__FILE__, __LINE__, Base::Log::LogLevel::kLogCritical,        \
                 __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

void LogWritePlugin(const char *file, int line, int category,
                    const char *format, ...);

#ifdef __cplusplus
};     /* extern "C" */
#endif /*__cplusplus*/
