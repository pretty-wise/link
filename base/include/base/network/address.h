/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include <netdb.h>

namespace Base {

class Url;

namespace Socket {

class Address {
public:
  static bool CreateUDP(const char *hostname, u16 port, Address *result);
  static bool CreateUDP(const Base::Url &url, Address *result);
  static bool CreateUDP(const char *hostname, const char *service,
                        Address *result);

  static bool CreateTCP(const Base::Url &url, Address *result);
  static bool CreateTCP(const char *hostnam, const char *service,
                        Address *result);

  bool operator==(const Address &rhs) const;
  bool operator!=(const Address &rhs) const;

  bool GetHostname(char *buffer, u32 nbytes) const;
  bool GetService(char *buffer, u32 nbytes) const;

public:
  struct sockaddr_storage m_data;
  u32 m_length;
};

const char *Print(const Address &addr);

} // namespace Socket

///////////////////////////////////////////////////////////////////////////////

s32 NetToHostL(s32 value);
s16 NetToHostS(s16 value);
s32 HostToNetL(s32 value);
s16 HostToNetS(s16 value);

class AddressIPv4 {
public:
  static const AddressIPv4 kLocalhost;
  static const AddressIPv4 kAny;

public:
  inline AddressIPv4() : m_address(0) {}
  inline AddressIPv4(u32 addr) : m_address(addr) {}
  inline AddressIPv4(u8 a, u8 b, u8 c, u8 d)
      : m_address((a << 24) | (b << 16) | (c << 8) | d) {}

  static bool FromString(AddressIPv4 *address, const char *hostname);

  inline u8 GetA() const { return (m_address & 0xff000000) >> 24; }
  inline u8 GetB() const { return (m_address << 8) >> 24; }
  inline u8 GetC() const { return (m_address << 16) >> 24; }
  inline u8 GetD() const { return (m_address & 0x000000ff); }

  inline u32 GetRaw() const { return m_address; }

  inline bool operator==(const AddressIPv4 &rhs) const {
    return m_address == rhs.m_address;
  }
  inline bool operator!=(const AddressIPv4 &rhs) const {
    return m_address != rhs.m_address;
  }

private:
  // host byte order.
  u32 m_address;
};

} // namespace Base
