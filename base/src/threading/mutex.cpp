/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/threading/mutex.h"
#include "base/core/assert.h"

namespace Base {

void Mutex::Initialize() {
  // init attributes
  pthread_mutexattr_t attrib;
  pthread_mutexattr_init(&attrib);

  // recursive
  pthread_mutexattr_settype(&attrib, PTHREAD_MUTEX_RECURSIVE);

  // create
  int res = pthread_mutex_init(&m_handle, &attrib);
  BASE_ASSERT(res == 0, "mutex init failed");

  // release attributes
  pthread_mutexattr_destroy(&attrib);
}

void Mutex::Terminate() {
  int res = pthread_mutex_destroy(&m_handle);
  BASE_ASSERT(res == 0, "mutex destroy failed");
}

void Mutex::Lock() {
  int res = pthread_mutex_lock(&m_handle);
  BASE_ASSERT(res == 0, "mutex lock failed");
}

void Mutex::Unlock() {
  int res = pthread_mutex_unlock(&m_handle);
  BASE_ASSERT(res == 0, "mutex unlock failed");
}

} // namespace Base
