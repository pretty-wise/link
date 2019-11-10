/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"

namespace Base {

class Time {
public:
  // returns current time in milliseconds.
  static u32 GetTimeMs();

  // returns current time in nanoseconds.
  static u64 GetTimeUs();

  static u64 GetElapsedUs(u64 since);
  static u32 GetElapsedMs(u32 since);
  static u64 GetCycleCount();
  static u32 GetFrequency();
};

} // namespace Base
