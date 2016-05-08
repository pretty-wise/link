/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include "base/threading/condition_variable.h"
#include "base/threading/mutex.h"

#include <list>

struct Notification;

namespace Link {

class NotificationQueue {
public:
  NotificationQueue();
  ~NotificationQueue();
  void Push(const Notification &notif);
  bool Pop(Notification &notif, int timeout_ms);

private:
  typedef std::list<Notification> NotificationList;
  NotificationList m_list;
  Base::ConditionVariable m_cond;
  Base::Mutex m_lock;
};

} // namespace Link
