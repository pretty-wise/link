/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"

#include <unistd.h>
#include <pthread.h>

namespace Base {

class Mutex {
public:
  void Initialize();
  void Terminate();

  void Lock();
  void Unlock();

  friend class ConditionVariable;

private:
  pthread_mutex_t m_handle;
};

class MutexAutoLock {
public:
  MutexAutoLock(Mutex &mutex) : m_mutex(mutex) { m_mutex.Lock(); }
  ~MutexAutoLock() { m_mutex.Unlock(); }

private:
  MutexAutoLock &operator=(const MutexAutoLock &rhs);

  Mutex &m_mutex;
};

} // namespace Base
