/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/network/address.h"
#include "base/network/url.h"
#include "base/core/assert.h"
#include "base/core/memory.h"
#include "base/core/atomic.h"

#include <netdb.h>
#include <arpa/inet.h>

namespace Base {

namespace Socket {

const char *Print(const Address &addr) {
  const u32 kNumBuffers = 32;
  const socklen_t addr_len = 2 * INET6_ADDRSTRLEN;
  __thread static char addr_str_buffers[addr_len][kNumBuffers];
  __thread static std::atomic_ushort index(kNumBuffers);
  while(index >= kNumBuffers) {
    ++index;
  }

  char *addr_str = addr_str_buffers[index];
  memset(addr_str, 0, addr_len);
  addr.GetHostname(addr_str, addr_len);

  u32 i = 0;
  while(addr_str[i] != '\0') {
    ++i;
  }

  addr_str[i++] = ':';
  addr.GetService(&addr_str[i], addr_len - i);
  return addr_str;
}

bool Address::GetHostname(char *buffer, u32 nbytes) const {
  void *addrin = m_data.ss_family == AF_INET
                     ? (void *)&(((struct sockaddr_in *)&m_data)->sin_addr)
                     : (void *)&(((struct sockaddr_in6 *)&m_data)->sin6_addr);
  const char *res = inet_ntop(m_data.ss_family, addrin, buffer, nbytes);
  return res != nullptr;
}

bool Address::GetService(char *buffer, u32 nbytes) const {
  u16 port = m_data.ss_family == AF_INET
                 ? ((struct sockaddr_in *)&m_data)->sin_port
                 : ((struct sockaddr_in6 *)&m_data)->sin6_port;
  int res = snprintf(buffer, nbytes, "%d", port);
  return res >= 0 && res <= nbytes;
}

enum Type { kUDP, kTCP };

bool Create(Type type, const char *hostname, const char *service,
            struct sockaddr_storage *addr, u32 *length) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = type == Type::kUDP ? SOCK_DGRAM : SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = type == kUDP ? IPPROTO_UDP : IPPROTO_TCP;

  struct addrinfo *results;
  int err = getaddrinfo(hostname, service, &hints, &results);
  if(err) {
    return false;
  }
  for(struct addrinfo *p = results; p != NULL; p = p->ai_next) {
    memcpy(addr, p->ai_addr, sizeof(struct sockaddr));
    *length = p->ai_addrlen;
    freeaddrinfo(results);
    return true;
  }
  freeaddrinfo(results);
  return false;
}

bool Address::CreateUDP(const char *hostname, u16 port, Address *result) {
  char str_port[Base::Url::kServiceMax];
  snprintf(str_port, Base::Url::kServiceMax, "%d", port);
  return CreateUDP(hostname, str_port, result);
}

bool Address::CreateUDP(const Base::Url &url, Address *result) {
  return CreateUDP(url.GetHostname(), url.GetService(), result);
}

bool Address::CreateUDP(const char *hostname, const char *service,
                        Address *result) {
  return Create(kUDP, hostname, service, &result->m_data, &result->m_length);
}

bool Address::CreateTCP(const Base::Url &url, Address *result) {
  return CreateTCP(url.GetHostname(), url.GetService(), result);
}

bool Address::CreateTCP(const char *hostname, const char *service,
                        Address *result) {
  return Create(kTCP, hostname, service, &result->m_data, &result->m_length);
}

bool Address::operator==(const Address &rhs) const {
  return m_length == rhs.m_length &&
         0 == memcmp(&m_data, &rhs.m_data, m_length);
}

bool Address::operator!=(const Address &rhs) const { return !(*this == rhs); }

} // namespace Socket

const AddressIPv4 AddressIPv4::kLocalhost(127, 0, 0, 1);
const AddressIPv4 AddressIPv4::kAny(0, 0, 0, 0);

bool AddressIPv4::FromString(AddressIPv4 *address, const char *hostname) {
  BASE_ASSERT(address);
  struct hostent *value = gethostbyname(hostname);
  if(!value) {
    return false;
  }
  address->m_address = ntohl(((struct in_addr *)value->h_addr_list[0])->s_addr);
  return true;
}

s32 NetToHostL(s32 value) { return ntohl(value); }
s16 NetToHostS(s16 value) { return ntohs(value); }
s32 HostToNetL(s32 value) { return htonl(value); }
s16 HostToNetS(s16 value) { return ntohl(value); }

} // namespace Base
