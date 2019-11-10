/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/network/address.h"
#include "base/core/types.h"

#include <stdio.h>
#include <cstring>
#include <string.h>

#define PRINTF_URL(url) url.GetHostname(), url.GetService()

namespace Base {

namespace Socket {
class Address;
}

class Url {
public:
  static const u32 kHostnameMax = 128;
  static const u32 kServiceMax = 64;

  Url();
  Url(const AddressIPv4 &address, u16 port);
  Url(const char *string);
  Url(const char *address, u16 port);
  Url(const char *hostname, const char *service);
  Url(const Socket::Address &addr);

  const char *GetHostname() const { return m_hostname; }
  const char *GetService() const { return m_service; }

  u16 GetPort() const;

  inline bool operator==(const Url &rhs) const {
    return strncmp(m_hostname, rhs.m_hostname, kHostnameMax) &&
           strncmp(m_service, rhs.m_service, kServiceMax);
  }
  inline bool operator!=(const Url &rhs) const {
    return strncmp(m_hostname, rhs.m_hostname, kHostnameMax) ||
           strncmp(m_service, rhs.m_service, kServiceMax);
  }

private:
  char m_hostname[kHostnameMax];
  char m_service[kServiceMax];
};

} // namespace Base
