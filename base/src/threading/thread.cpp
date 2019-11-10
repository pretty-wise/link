/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/threading/thread.h"
#include "base/core/assert.h"
#include <unistd.h>

namespace Base {

void *PosixThreadMainRoutine(void *data) {
  // Do some work here.

  Thread *thread = (Thread *)data;

  int res = thread->Execute();

  return reinterpret_cast<void *>(res);
}

int Thread::Execute() { return m_entry_point(m_data); }

bool Thread::Initialize(int (*entry_point)(void *arg), void *arg,
                        ThreadPriority::Enum priority) {
  m_entry_point = entry_point;

  m_data = arg;

  int res = pthread_attr_init(&m_attribs);
  BASE_ASSERT(res == 0, "problem initializing thread attribs");

  // set stack size.
  size_t stack_size;
  pthread_attr_getstacksize(&m_attribs, &stack_size);
  res = pthread_attr_setstacksize(&m_attribs, 2 * stack_size);
  BASE_ASSERT(res == 0, "problem setting stack size for thread attribs");

  // sched policy
  pthread_attr_setschedpolicy(&m_attribs, SCHED_FIFO);
  BASE_ASSERT(res == 0, "Could not set scheduling policy to fifo");

  res = pthread_create(&m_handle, &m_attribs, &PosixThreadMainRoutine, this);
  if(res != 0) {
    BASE_LOG_LINE("problem creating thread");
  }

  // TODO:
  // if( res == 0 )
  //		SetPriority(prio);

  return res == 0;
}

void Thread::Terminate() {
  int res = pthread_attr_destroy(&m_attribs);
  if(res != 0) {
    BASE_LOG_LINE("problem terminating thread.");
  }
}

void Thread::Sleep(u32 milliseconds) { usleep(milliseconds * 1000); }

void Thread::Join() {
  int res = pthread_join(m_handle, nullptr);
  BASE_ASSERT(res == 0, "failed pthread_join.");
}

} // namespace Base
