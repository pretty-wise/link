/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include "base/network/socket.h"

#include <functional>

namespace Base {

namespace Epoll {
enum EpollOperation {
  OP_READ = 0x1,
  OP_WRITE = 0x2,
  OP_ERROR = 0x4,
  OP_ALL = OP_READ | OP_WRITE | OP_ERROR
};

inline void ToString(int operations, char string[3]) {
  string[0] = (operations & OP_READ) == OP_READ ? 'r' : '-';
  string[1] = (operations & OP_WRITE) == OP_WRITE ? 'w' : '-';
  string[2] = (operations & OP_ERROR) == OP_ERROR ? 'e' : '-';
}
}

typedef int EpollHandle;

EpollHandle EpollCreate(u32 max_events);

void EpollDestroy(EpollHandle context);

bool EpollAdd(EpollHandle context, Socket::Handle socket, void *user_data,
              int operations);

bool EpollUpdate(EpollHandle context, Socket::Handle socket, void *user_data,
                 int opetations);

void EpollRemove(EpollHandle context, Socket::Handle);

int EpollWait(EpollHandle context, s32 timeout_ms, const s32 max_events,
              std::function<void(void *user_data, int operations,
                                 int debug_data)> result);

} // namespace Base
