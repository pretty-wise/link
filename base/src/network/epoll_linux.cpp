/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/network/epoll.h"
#include "base/core/macro.h"
#include <unistd.h>

#	include <sys/epoll.h>

namespace Base {

int EngineToEpollEventMask(int events) {
	int result = 0;
	result |= (events & Epoll::OP_READ) ? EPOLLIN : 0;
	result |= (events & Epoll::OP_WRITE) ? EPOLLOUT : 0;
	result |= (events & Epoll::OP_ERROR) ? EPOLLERR : 0;
	return result;
}

int EpollToEngineEventMask(int events) {
	int result = 0;
	result |= (events & EPOLLIN) ? Epoll::OP_READ : 0;
	result |= (events & EPOLLOUT) ? Epoll::OP_WRITE : 0;
	result |= (events & EPOLLERR) ? Epoll::OP_ERROR : 0;
	return result;
}

EpollHandle EpollCreate(u32 max_events) {
	EpollHandle res = epoll_create(max_events);

	if(res < 0) {
		BASE_LOG_LINE("error on epoll_create: %d", errno);
	}

	return res;
}

void EpollDestroy(EpollHandle context) {
	int res = close(context);
	if(res == -1) {
		BASE_LOG_LINE("error on epoll close: %d", errno);
	}
}

bool EpollAdd(EpollHandle context, Socket::Handle socket, void* user_data, int operations) {
	epoll_event event;
	event.events = EngineToEpollEventMask(operations) | EPOLLET;
	event.data.ptr = user_data;

	int res = epoll_ctl(context, EPOLL_CTL_ADD, socket, &event);
	if(res != 0) {
		BASE_LOG_LINE("epoll_ctl EPOLL_CTL_ADD failed with %d", errno);
	}
	return res == 0;
}

bool EpollUpdate(EpollHandle context, Socket::Handle socket, void* user_data, int operations) {
	epoll_event event;
	event.events = EngineToEpollEventMask(operations) | EPOLLET;
	event.data.ptr = user_data;

	int res = epoll_ctl(context, EPOLL_CTL_MOD, socket, &event);
	if(res != 0) {
		BASE_LOG_LINE("epoll_ctl EPOLL_CTL_MOD failed with %d", errno);
	}
	return res == 0;
}

void EpollRemove(EpollHandle context, Socket::Handle socket) {
	int res = epoll_ctl(context, EPOLL_CTL_DEL, socket, NULL);
	if(res != 0) {
		BASE_LOG_LINE("epoll_ctl EPOLL_CTL_DEL failed with %d", errno);
	}
}

int EpollWait(EpollHandle context, s32 timeout_ms, const s32 max_events, std::function<void(void* user_data, int operations, int debug_data)> result) {
	epoll_event events[max_events];

	int num_ready = epoll_wait(context, events, max_events, timeout_ms);
	if(num_ready < 0) {
		BASE_LOG_LINE("epoll_wait failed with %d", errno);
	}
	
	for(int i = 0; i < num_ready; ++i) {
		epoll_event& evt = events[i];
		int operations = EpollToEngineEventMask(evt.events);
//		BASE_LOG_LINE("epoll_wait event for %d native %d engine %d", (s64)evt.data.ptr, evt.events, operations);
		result(evt.data.ptr, operations, 0);
	}

	return num_ready;
}

} // namespace Base
