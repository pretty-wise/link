/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/core/time_utils.h"
#include "base/core/assert.h"

#if defined __linux__
#include <time.h>
#else
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

namespace Base {

#if defined __APPLE__
double to_us_ratio() {
  mach_timebase_info_data_t info;
  (void)mach_timebase_info(&info);

  double ratio = (double)info.numer / (double)info.denom;

  return ratio;
}

double to_ms_ratio() {
  /*
          mach_timebase_info_data_t info;
          (void)mach_timebase_info(&info);

          float ratio = (float)info.numer / (float)info.denom / 1000000.0;

          return ratio;
  */
  return to_us_ratio() * 0.000001;
}
#endif

u32 Time::GetTimeMs() {
#if defined __APPLE__
  u64 current = mach_absolute_time();

  static double ratio = to_ms_ratio();

  return (u32)(current * ratio);
#else
  struct timespec time;
  //	int res = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
  int res = clock_gettime(CLOCK_REALTIME, &time);
  BASE_ASSERT(res == 0, "faled to get time");
  time_t milliseconds = time.tv_sec * 1000 + time.tv_nsec / 1000000;
  //	BASE_LOG("time %d %d %ld ", time.tv_sec, time.tv_nsec, milliseconds);
  return milliseconds;
#endif
}

u64 Time::GetTimeUs() {
#if defined __APPLE__
  u64 current = mach_absolute_time();

  static double ratio = to_us_ratio();

  return current * ratio;
#else
  return 0;
#endif
}

u64 Time::GetElapsedUs(u64 since) {
  return GetTimeUs() - since; // todo: rewrite for better precision.
}

u32 Time::GetElapsedMs(u32 since) {
  return GetTimeMs() - since; // todo: rewrite for better precision.
}

u64 Time::GetCycleCount() {
#if defined __APPLE__
  return mach_absolute_time();
#else
  return 1;
#endif
}

u32 Time::GetFrequency() {
  return 1; // todo
}

} // namespace	Base
