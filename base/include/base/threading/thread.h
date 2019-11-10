/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include <cstddef>
#include <pthread.h>
#include <sys/types.h>

#include "base/core/types.h"

namespace Base {

namespace ThreadPriority {
enum Enum {
  Idle = -15,
  Lowest = -2,
  BelowNormal = -1,
  Normal = 0,
  AboveNormal = 1,
  Highest = 2,
  Critical = 15
};
}

class Thread {
public:
  typedef pthread_t Handle;

  static void Sleep(u32 milliseconds);

  Thread() : m_handle(0), m_entry_point(nullptr), m_data(nullptr) {}
  bool Initialize(int (*entry_point)(void *arg), void *arg,
                  ThreadPriority::Enum priority = ThreadPriority::Normal);

  void Terminate();

  void Join();

  friend void *PosixThreadMainRoutine(void *data);

private:
  int Execute();

  Handle m_handle;
  int (*m_entry_point)(void *);
  void *m_data;
  pthread_attr_t m_attribs;
};

} // namespace Base
