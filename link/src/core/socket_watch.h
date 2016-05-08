/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include "link/link.h"

#include <unistd.h>
#include <functional>

namespace Link {

class TcpSocket;

class SocketWatch {
public:
	SocketWatch(u32 num_sockets);
	~SocketWatch();

	bool Wait(time_ms timeout, std::function<void (ConnectionHandle connection, bool read,	bool write, bool error)> func);

	//bool Register(const TcpSocket& socket, ConnectionHandle handle);

private:
	int m_kqueue;
};

} // namespace Link
