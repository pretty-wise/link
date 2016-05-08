/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "notification_queue.h"
#include "link/link.h"
#include "base/core/macro.h"

namespace Link {

NotificationQueue::NotificationQueue() {
  m_lock.Initialize();
  m_cond.Initialize();
}

NotificationQueue::~NotificationQueue() {
  m_lock.Terminate();
  m_cond.Terminate();
}

void NotificationQueue::Push(const Notification &notif) {
  {
    Base::MutexAutoLock lock(m_lock);
    // BASE_LOG("pushing notif %d\n", notif.type);
    m_list.push_back(notif);
  }

  m_cond.Notify();
}

bool NotificationQueue::Pop(Notification &notif, int timeout_ms) {
  bool result = false;

  Base::MutexAutoLock lock(m_lock);

  if(0 == timeout_ms) {
    // if the user sleeps for zero time then we only check the queue and return
    result = !m_list.empty();
  } else {
    if(m_list.empty() /* && 0 == m_wake_up*/) {
      m_cond.Wait(m_lock, timeout_ms);
    }
  }

  // set the wakeup back to zero.
  // m_wake_up = 0;
  if(!m_list.empty()) {
    notif = m_list.front();
    m_list.pop_front();
    result = true;
  }
  /*
                  if(m_list.empty() && !notified) {
                          notified = m_cond.Wait(m_lock, timeout_ms);
                  }

                  if(!m_list.empty()) {
                          notif = m_list.front();
                          BASE_LOG("poping notif %d\n", notif.type);
                          m_list.pop_front();
                          return true;
                  }
          }
          // timedout
          return false;*/

  return result;
}

} // namespace Link
