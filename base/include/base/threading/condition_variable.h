/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"

#include <pthread.h>

namespace Base {

class Mutex;

class ConditionVariable {
public:
  ConditionVariable();
  ~ConditionVariable();

  void Initialize();
  void Terminate();

  bool Wait(Mutex &lock, time_ms timeout = 0);

  void Notify();

  void NotifyAll();

private:
  pthread_cond_t m_handle;
};

} // namespace Base
