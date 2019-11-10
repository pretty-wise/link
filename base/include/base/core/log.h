/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/log_channel.h"
#include "base/core/types.h"

#include <cstdarg>

#define BASE_DEBUG(channel, ...)                                               \
  Base::Log::Write(__FILE__, __LINE__, Base::Log::kLogDebug, channel,          \
                   __VA_ARGS__)
#define BASE_INFO(channel, ...)                                                \
  Base::Log::Write(__FILE__, __LINE__, Base::Log::kLogInfo, channel,           \
                   __VA_ARGS__)
#define BASE_WARN(channel, ...)                                                \
  Base::Log::Write(__FILE__, __LINE__, Base::Log::kLogWarning, channel,        \
                   __VA_ARGS__)
#define BASE_ERROR(channel, ...)                                               \
  Base::Log::Write(__FILE__, __LINE__, Base::Log::kLogError, channel,          \
                   __VA_ARGS__)
#define BASE_CRITICAL(channel, ...)                                            \
  Base::Log::Write(__FILE__, __LINE__, Base::Log::kLogCritical, channel,       \
                   __VA_ARGS__)

namespace Base {
namespace Log {

static const u32 kLogLineMaxLength = 4 * 1024;

enum LogLevel {
  kLogDebug = (0x1 << 0),    /*!< Debug category */
  kLogInfo = (0x1 << 1),     /*!< Informational category */
  kLogWarning = (0x1 << 2),  /*!< Warning category */
  kLogError = (0x1 << 3),    /*!< Recoverable error category */
  kLogCritical = (0x1 << 4), /*!< Non-recoverable error category */
  kLogAll = kLogDebug | kLogInfo | kLogWarning | kLogError | kLogCritical
};

const LogChannel kDefaultCategory("default");
const LogChannel kAnyCategory("");

void Write(const char *file, int line, int level, const LogChannel &category,
           const char *format, ...);

void Write(const char *file, int line, int level, const LogChannel &channel,
           const char *format, va_list arg_list);

typedef void (*LogHook)(int level, const char *log_line, int length,
                        void *context);

void ConsoleOutput(int level, const char *log_line, int length, void *context);

int Register(int filter, LogHook callback, void *context);
int Register(const LogChannel &channel, int filter, LogHook callback,
             void *context);
void Unregister(int handle);

} // namespace Log
} // namespace Base
