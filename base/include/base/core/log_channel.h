/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/math/crc.h"

namespace Base {

class LogChannel {
public:
  LogChannel(const char *name) : m_name(name), m_crc(Math::crc(name)) {}

  const char *GetName() const { return m_name; }

  bool operator==(const LogChannel &rhs) const { return m_crc == rhs.m_crc; }

private:
  const char *m_name;
  u32 m_crc;
};

} // namespace Base
