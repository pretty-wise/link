/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/threading/condition_variable.h"

#include "base/threading/mutex.h"
#include "base/core/assert.h"

#include <sys/time.h>
#include <errno.h>

namespace Base {

ConditionVariable::ConditionVariable() {}

ConditionVariable::~ConditionVariable() {}

void ConditionVariable::Initialize() {
  int ret = pthread_cond_init(&m_handle, NULL);
  BASE_ASSERT(ret == 0);
}

void ConditionVariable::Terminate() {
  int ret = pthread_cond_destroy(&m_handle);
  BASE_ASSERT(ret == 0);
}

bool ConditionVariable::Wait(Mutex &lock, time_ms timeout_ms) {
  int result = 0;
  if(timeout_ms < 0) {
    result = pthread_cond_wait(&m_handle, &lock.m_handle);
    BASE_ASSERT(result == 0);
  } else {

    int kMsecPerSec = 1000;
    long kNsecPerMsec = 1000000;
    long kNsecPerSec = 1000000000;
    int kUsecPerMsec = 1000;
    // long kUsecPerSec = 1000000;
    timespec relative_time = {timeout_ms / kMsecPerSec,
                              (timeout_ms % kMsecPerSec) * kNsecPerMsec};

    struct timeval now;
    gettimeofday(&now, NULL);
    struct timespec absolute_time = {now.tv_sec, now.tv_usec / kUsecPerMsec};
    absolute_time.tv_sec +=
        relative_time.tv_sec + relative_time.tv_nsec / kNsecPerSec;
    absolute_time.tv_nsec =
        (absolute_time.tv_nsec + relative_time.tv_nsec) % kMsecPerSec;

    result = pthread_cond_timedwait(&m_handle, &lock.m_handle, &absolute_time);
    BASE_ASSERT(result == 0 || result == ETIMEDOUT);
  }

  return result == 0;
}

void ConditionVariable::Notify() {
  int res = pthread_cond_signal(&m_handle);
  BASE_ASSERT(res == 0);
}

void ConditionVariable::NotifyAll() {
  int res = pthread_cond_broadcast(&m_handle);
  BASE_ASSERT(res == 0);
}

} // namespace Base
