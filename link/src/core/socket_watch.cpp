/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "SocketWatch.h"
#include "base/network/socket.h"

#include "base/core/macro.h"
#include "base/threading/thread.h"

#if defined __APPLE__
#	include <sys/types.h>
#	include <sys/event.h>
#	include <sys/time.h>
#endif

#include <unistd.h>

namespace Link {

SocketWatch::SocketWatch(u32 num_sockets) {
#if defined __APPLE__
	m_kqueue = kqueue();
#endif
	BASE_ASSERT(m_kqueue != -1);
}

SocketWatch::~SocketWatch() {
#if defined __APPLE__	
	close(m_kqueue);
#endif
}
/*
bool SocketWatch::Register(const TcpSocket& socket, ConnectionHandle handle) {
#if defined __APPLE__
	struct kevent event;

	// EFILT_READ - if listen() read is returned if there is a connection to accept. data contains the size of backlog.
	// EFILT_READ - other - data to read.
	// EFILT_WRITE - data may be written.


	EV_SET(&event, // event to initialize
		socket.GetHandle(),	// file to watch
		EVFILT_READ | EVFILT_WRITE, // events to catch
		EV_ADD,// | EV_EOF, // general flags
		0, // filter specific flags
		0, // filter specific data
		handle // user data pointer
	);

	int res = kevent( m_kqueue, &event, 1, nullptr, 0, nullptr );

//	BASE_LOG("register for %d returned %d\n", socket.GetHandle(), res);

	return res != -1;
#else
	return false;
#endif
}
*/
bool SocketWatch::Wait(time_ms timeout, std::function<void (ConnectionHandle connection, bool read,	bool write, bool error)> func) {
#if defined __APPLE__
	__darwin_time_t timeout_sec = timeout / 1000;
	long timeout_us = ((timeout - timeout_sec*1000) * 1000000);
	struct timespec timeoutspec = { timeout_sec, timeout_us }; // seconds, nanoseconds.
	//BASE_LOG("%d - %d:%d", timeout, timeout_sec, timeout_us);

	const int max_events = 20;

	struct kevent response_list[max_events];

		// add events for every existing socket.
		for( u16 i = 0; i < /*m_connection_count+*/1; ++i )
		{
				// event to init, event id, kernel filter, action to perform on event, filter spec flags, filter spec data, user data.

				// notify when data available to read. on return 'data' contains the number of bytes ready for reading.
				//EV_SET(&monitor_list[i], m_sockets[i].GetHandle(), EVFILT_READ, EV_ADD, 0, 0, nullptr);
		}


		// block until events available.
		signed int num_events = kevent( m_kqueue, nullptr, 0, response_list, max_events, &timeoutspec );

		if( num_events == -1 )
		{
				BASE_LOG("failed on kevent\n");
		}
		else if( num_events == 0 )
		{
				// no events after timeout.
				//BASE_LOG("kevent timeout\n");
				return false;
		}
		else
		{
				for( int i = 0; i < num_events; ++i )
				{
					const struct kevent& event = response_list[i];

					bool read = false, write = false, error = false;
					error = (event.flags & EV_ERROR) || (event.flags & EV_EOF);
					read = event.flags & EVFILT_READ;
					write = event.flags & EVFILT_WRITE;

					func((ConnectionHandle)response_list[i].udata, read, write, error);
				}
		}


	return true;
#else
	return false;
#endif
}

} // namespace Link
