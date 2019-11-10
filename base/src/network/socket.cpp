/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
//
//	Socket.cpp
//	Base
//
//	Created by Krzysiek Stasik on 05/07/14.
//	Copyright (c) 2014 Krzysiek Stasik. All rights reserved.
//

#include "base/core/assert.h"
#include "base/network/socket.h"

#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace Base {
namespace Socket {

void Close(Handle socket) {
  int res = close(socket);
  BASE_ASSERT(res != -1, "error: %d", errno);
}

const char *ErrorToString(int error) {
  switch(error) {
  case EACCES:
    return "EACCES";
  case EAGAIN:
    return "EAGAIN";
  // case EWOULDBLOCK: return "EWOULDBLOCK";
  case EBADF:
    return "EBADF";
  case ECONNRESET:
    return "ECONNRESET";
  case EDESTADDRREQ:
    return "EDESTADDRREQ";
  case EFAULT:
    return "EFAULT";
  case EINTR:
    return "EINTR";
  case EINVAL:
    return "EINVAL";
  case EISCONN:
    return "EISCONN";
  case EMSGSIZE:
    return "EMSGSIZE";
  case ENOBUFS:
    return "ENOBUFS";
  case ENOMEM:
    return "ENOMEM";
  case ENOTCONN:
    return "ENOTCONN";
  case ENOTSOCK:
    return "ENOTSOCK";
  case EOPNOTSUPP:
    return "EOPNOTSUPP";
  case EPIPE:
    return "EPIPE";
  }
  return "UNKNOWN";
}

bool GetBoundAddress(Handle socket, Base::Url *url) {
  struct sockaddr_in address;
  memset(&address, 0, sizeof(sockaddr));
  socklen_t addr_len = sizeof(address);
  int res = getsockname(socket, (struct sockaddr *)&address, &addr_len);

  if(res == -1 || addr_len != sizeof(address)) {
    return false;
  }

  // todo(kstasik):
  // url->SetAddress(Base::AddressIPv4((u32)ntohl(address.sin_addr.s_addr)));
  // url->SetPort(ntohs(address.sin_port));
  return true;
}

bool GetNetworkInterface(Base::AddressIPv4 *address, const char *if_name) {
  struct ifaddrs *ifap;
  bool result = false;
  if(-1 == getifaddrs(&ifap)) {
    return false;
  }

  // todo(kstasik): very naive way, binds to first AF_INET eth0 interface...
  for(struct ifaddrs *ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next) {
    if(ifa->ifa_addr == nullptr)
      continue;
    if(!(ifa->ifa_flags & IFF_UP))
      continue;

    if(ifa->ifa_addr->sa_family == AF_INET) {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)ifa->ifa_addr;
      if(result == false &&
         (0 == strncmp(ifa->ifa_name, if_name, strlen(if_name)))) {
        result = true;
        *address = Base::AddressIPv4((u32)ntohl(ipv4->sin_addr.s_addr));
      }
    }
  }

  freeifaddrs(ifap);
  return result;
}

namespace Tcp {

Handle Open() {
  int error;
  return Open(&error);
}

Handle Open(int *error) {
  Handle handle = socket(PF_INET, SOCK_STREAM, 0);
  if(handle == InvalidHandle) {
    *error = errno;
    return handle;
  }

  // reuse address
  const int yes = 1;
  if(-1 == setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
    Close(handle);
    *error = -1;
    return InvalidHandle;
  }

  // non-blocking socket.
  if(-1 == fcntl(handle, F_SETFL, O_NONBLOCK)) {
    *error = -2;
    Close(handle);
    return InvalidHandle;
  }
  return handle;
}

bool Listen(Handle socket, u16 *port) {
  int error;
  return Listen(socket, port, &error);
}

bool Listen(Handle socket, u16 *port, int *error) {
  if(socket == InvalidHandle) {
    *error = (int)-1;
    return false;
  }

  struct sockaddr_in address;
  memset(&address, 0, sizeof(sockaddr));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(*port);

  int res = bind(socket, (const struct sockaddr *)&address, sizeof(address));

  if(res == -1) {
    *error = errno;
    BASE_LOG("bind on port %d failed with %d\n", *port, errno);
    return false;
  }

  socklen_t addr_len = sizeof(address);
  res = getsockname(socket, (struct sockaddr *)&address, &addr_len);

  if(res == -1) {
    *error = -2;
    return false;
  }

  *port = ntohs(address.sin_port);

  res = listen(socket, 5);

  if(res == -1) {
    *error = -3;
    return false;
  }

  return true;
}

int Connect(Handle socket, const Address &url) {
  int result = connect(socket, (struct sockaddr *)&url.m_data, url.m_length);
  BASE_ASSERT(result == 0 || (result == -1 && errno == EINPROGRESS),
              "socket connect failed %d. errno: %d.", result, errno);
  if(result == 0) {
    return kConnected;
  } else if(result == -1 && errno == EINPROGRESS) {
    return kConnecting;
  }
  return kFailed;
  // return result == 0 || (result == -1 && errno == EINPROGRESS);
}

int IsConnected(Handle socket) {
  struct timeval timeout = {0, 0};
  fd_set read_set;
  FD_ZERO(&read_set);
  FD_SET(socket, &read_set);

  int result = select(socket + 1, nullptr, &read_set, nullptr, &timeout);
  if(result < 0 && errno != EINTR) {
    return kFailed;
  } else if(result > 0) {
    int valopt;
    socklen_t len = sizeof(int);
    if(getsockopt(socket, SOL_SOCKET, SO_ERROR, (void *)&valopt, &len) < 0) {
      BASE_LOG_LINE("getsockopt %d", valopt);
      if(valopt == EINPROGRESS) {
        return kConnecting;
      } else if(valopt == 0) {
        return kConnected;
      } else {
        return kFailed;
      }
    }
  } else {
    return kConnecting;
  }

  if(result == 1) {
    return kConnected;
  }
  return kFailed;
}

bool Accept(Handle socket, Handle *incoming, Address *connectee, u32 timeout_ms,
            int *error) {
  fd_set readset;
  FD_ZERO(&readset);
  FD_SET(socket, &readset);
  struct timeval timeout;
  timeout.tv_sec = timeout_ms / 1000;
  timeout.tv_usec = (timeout_ms % 1000) * 1000;
  int result = select(socket + 1, &readset, NULL, NULL, &timeout);
  if(result == 0) {
    *error = errno;
    return false;
  }

  return Accept(socket, incoming, connectee, error);
}
bool Accept(Handle socket, Handle *incoming, Address *connectee) {
  int error;
  return Accept(socket, incoming, connectee, &error);
}

bool Accept(Handle socket, Handle *incoming, Address *connectee, int *error) {
  socklen_t address_len = sizeof(struct sockaddr_in);

  int result =
      accept(socket, (struct sockaddr *)&connectee->m_data, &address_len);
  if(result < 0) {
    *error = errno;
    return false;
  }

  if(-1 == fcntl(result, F_SETFL, O_NONBLOCK)) {
    *error = errno;
    Close(result);
    return false;
  }

  const int yes = 1;
  if(-1 == setsockopt(result, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
    *error = errno;
    Close(result);
    return false;
  }

  *incoming = result;
  return true;
}

streamsize Send(Handle socket, const void *data, streamsize nbytes,
                int *error) {
  streamsize result = send(socket, data, nbytes, 0);
  if(result == -1) {
    *error = errno;
  }
  return result;
}

streamsize Recv(Handle socket, void *buffer, streamsize nbytes, int *error) {
  streamsize nbytes_read = read(socket, buffer, nbytes);
  if(nbytes_read == -1) {
    if(errno != EWOULDBLOCK) {
      BASE_LOG_LINE("socket read on %d failed with %d", socket, errno);
    }
    *error = errno;
  }
  return nbytes_read;
}

} // namespace Tcp

namespace Udp {

Handle Open(const Base::Url &url, u16 *port) {
  const char *hostname = url.GetHostname();
  const char *service = url.GetService();

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = 0;
  hints.ai_protocol = IPPROTO_UDP;

  struct addrinfo *result;
  int err = getaddrinfo(hostname, service, &hints, &result);
  if(err) {
    return InvalidHandle;
  }
  Handle handle = InvalidHandle;
  for(struct addrinfo *p = result; p != NULL; p = p->ai_next) {
    handle = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if(handle == InvalidHandle) {
      BASE_LOG_LINE("socket failed with %d", errno);
      continue;
    }
    int res = bind(handle, p->ai_addr, p->ai_addrlen);
    if(-1 == res) {
      BASE_LOG_LINE("bind failed with %d", errno);
      /*char host[128];
      char serv[128];
      if(0 == getnameinfo(p->ai_addr, p->ai_addrlen, host, 128, serv, 128, 0))
      {
        BASE_LOG_LINE("bind failed for %s:%s", host, serv);
      }*/
      Close(handle);
      continue;
    }
    //
    // non-blocking socket.
    if(-1 == fcntl(handle, F_SETFL, O_NONBLOCK)) {
      BASE_LOG_LINE("fcntl failed with %d", errno);
      Close(handle);
      continue;
    }

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    res = getsockname(handle, (struct sockaddr *)&addr, &addr_len);

    if(res == -1) {
      BASE_LOG_LINE("getsockname failed with %d", errno);
      Close(handle);
      continue;
    }

    *port = ntohs(addr.sin_port);
    break;
  }

  freeaddrinfo(result);
  return handle;
}

Handle Open(u32 address, u16 *port) {
  struct sockaddr_in my_addr;
  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(*port);
  my_addr.sin_addr.s_addr = htonl(address);

  Handle handle = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(handle == InvalidHandle) {
    BASE_LOG_LINE("socket failed with %d", errno);
    return InvalidHandle;
  }
  if(-1 == bind(handle, (struct sockaddr *)&my_addr, sizeof(my_addr))) {
    BASE_LOG_LINE("bind failed with %d", errno);
    Close(handle);
    return InvalidHandle;
  }

  // non-blocking socket.
  if(-1 == fcntl(handle, F_SETFL, O_NONBLOCK)) {
    BASE_LOG_LINE("fcntl failed with %d", errno);
    Close(handle);
    return InvalidHandle;
  }

  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);
  int res = getsockname(handle, (struct sockaddr *)&addr, &addr_len);

  if(res == -1) {
    BASE_LOG_LINE("getsockname failed with %d", errno);
    Close(handle);
    return InvalidHandle;
  }

  *port = ntohs(addr.sin_port);

  return handle;
}

bool Send(Handle socket, const Address &dest, const void *buffer,
          streamsize nbytes, int *error) {
  streamsize sent = sendto(socket, buffer, nbytes, 0, (sockaddr *)&dest.m_data,
                           dest.m_length);
  if(sent == -1) {
    *error = errno;
    return false;
  }
  return true;
}

streamsize Recv(Handle socket, Address *from, void *buffer, streamsize nbytes,
                int *error) {
  streamsize received = recvfrom(socket, (char *)buffer, nbytes, 0,
                                 (sockaddr *)&from->m_data, &from->m_length);
  if(received == -1) {
    if(errno == EWOULDBLOCK) {
      return 0;
    }
    BASE_LOG_LINE("socket read on %d failed with %d", socket, errno);
    *error = errno;
  }
  return received;
}

} // namespace Udp

} // namespace Socket
} // namespace Base
