/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include "base/network/url.h"

#include <errno.h>
#include <sys/socket.h>

#define PRINTF_ADDR(addr)

namespace Base {

namespace Socket {

typedef int Handle;
static const Handle InvalidHandle = -1;

const char *ErrorToString(int error);

void Close(Handle socket);

bool GetNetworkInterface(Base::AddressIPv4 *address,
                         const char *interface_name);

bool GetBoundAddress(Handle socket, Base::Url *url);

namespace Tcp {

static const int kFailed = 0;
static const int kConnecting = 1;
static const int kConnected = 2;

Handle Open();
Handle Open(int *error);
bool Listen(Handle socket, u16 *port);
bool Listen(Handle socket, u16 *port, int *error);
int Connect(Handle socket, const Address &addr);
int IsConnected(Handle socket);

bool Accept(Handle socket, Handle *incoming, Address *connectee, u32 timeout_ms,
            int *error);
bool Accept(Handle socket, Handle *incoming, Address *connectee);
bool Accept(Handle socket, Handle *incoming, Address *connectee, int *error);
streamsize Send(Handle socket, const void *data, streamsize nbytes, int *error);
streamsize Recv(Handle socket, void *buffer, streamsize nbytes, int *error);

} // namespace Tcp

namespace Udp {

Handle Open(u32 addr, u16 *port);
Handle Open(const Url &url, u16 *port);
inline Handle Open(u16 *port) { return Open((u32)0, port); }
bool Send(Handle socket, const Address &addr, const void *buffer,
          streamsize nbytes, int *error);
streamsize Recv(Handle socket, Address *addr, void *buffer, streamsize nbytes,
                int *error);

} // namespace Udp

} // namespace Socket
} // namespace Base
