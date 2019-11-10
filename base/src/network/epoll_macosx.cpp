/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/network/epoll.h"
#include "base/core/macro.h"
#include <unistd.h>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>			 
#include <errno.h>

namespace Base {

int KqueueToEngine(int filter) {
	if(filter == EVFILT_READ)
		return Epoll::OP_READ;
	if(filter == EVFILT_WRITE)
		return Epoll::OP_WRITE;

	return 0;
}

EpollHandle EpollCreate(u32 max_events) {
	EpollHandle result = kqueue();
	return result;
}

void EpollDestroy(EpollHandle context) {
	close(context);
}

bool EpollAdd(EpollHandle context, Socket::Handle socket, void* user_data, int operations) {
	struct kevent ev;

	bool result = true;

	// EV_CLEAR - for edge-triggered mode.

	if(operations & Epoll::OP_READ) {
		EV_SET(&ev, socket, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, user_data);

		result |= 0 == kevent(context, &ev, 1, NULL, 0, 0);
		if(!result) {
			BASE_LOG_LINE("kevent failed with %d", errno);	
		}
	}

	if(operations & Epoll::OP_WRITE) {
		EV_SET(&ev, socket, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, user_data);

		result |= 0 == kevent(context, &ev, 1, NULL, 0, 0);
		if(!result) {
			BASE_LOG_LINE("kevent failed with %d", errno);	
		}
	}

	return result;
}

bool EpollUpdate(EpollHandle context, Socket::Handle socket, void* user_data, int operations) {
	EpollRemove(context, socket);
	return EpollAdd(context, socket, user_data, operations);
}

void EpollRemove(EpollHandle context, Socket::Handle socket) {
	struct kevent ev;
	EV_SET(&ev, socket, EVFILT_READ, EV_DELETE, 0, 0, 0);
	int res = kevent(context, &ev, 1, NULL, 0, 0);
	if(res == -1) {
		BASE_LOG_LINE("kevent EV_DELETE failed with %d errno %d.", res, errno);	
	}

	EV_SET(&ev, socket, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
	res = kevent(context, &ev, 1, NULL, 0, 0);
	// epoll sockets might not have EVFILT_WRITE registered.
	if(res == -1) {
		//BASE_LOG_LINE("kevent EV_DELETE failed with %d errno %d.", res, errno);	
	}
}

int EpollWait(EpollHandle context, s32 timeout_ms, const s32 max_events, std::function<void(void* user_data, int operations, int debug_data)> result) {
	struct kevent events[max_events];
	/*
			struct kevent {
						 uintptr_t			 ident;					 identifier for this event (socket handle)
						 int16_t				 filter;					filter for event (EVFILT_READ, EVFILT_WRITE)
						 uint16_t				flags;					 general flags (EV_ADD, EV_DELETE, EV_EOF)
						 uint32_t				fflags;					filter-specific flags
						 intptr_t				data;						filter-specific data
						 void						*udata;					opaque user data identifier
		 };
	*/
	
	// ms to timespec(sec, ns).
	const timespec timeout = { timeout_ms / 1000, (timeout_ms % 1000) * 1000000 };
	int num_ready = kevent(context, NULL, 0, events, max_events, &timeout);
	
	for(int i = 0; i < num_ready; ++i) {
		struct kevent& evt = events[i];
//		BASE_LOG_LINE("kevent on socket %d. filter %p, flags %p, fflags %p, data %d, udata %d\n", 
//				evt.ident, evt.filter, evt.flags, evt.fflags, evt.data, evt.udata);
		result(evt.udata, KqueueToEngine(events[i].filter), evt.ident);
	}

	return num_ready;
}

} // namespace Base
