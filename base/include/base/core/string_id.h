/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include <map>
#include <string>

#include "base/core/types.h"
#include "base/core/assert.h"
#include "base/core/str.h"
#include "base/math/crc.h"

#define STRINGID_STORE_STRING_VALUES

namespace Base {

class StringId {
public:
  typedef u32 Type;
  static const StringId Invalid;

  inline StringId();
  inline StringId(Type val);
  inline StringId(const char *string);
  inline bool IsValid() const;
  inline bool operator==(const StringId &rhs) const;
  inline bool operator!=(const StringId &rhs) const;
  inline bool operator>(const StringId &rhs) const;
  inline bool operator<(const StringId &rhs) const;
  const char *c_str() const { return m_string.c_str(); }
  Type GetValue() const { return m_value; }

private:
  Type m_value;

#if defined(STRINGID_STORE_STRING_VALUES)
  // todo: create buffer for this.
  std::string m_string;
  void RegisterString(StringId id, const char *value);
  static std::map<StringId, std::string> s_string_storage;
#endif
};

//------------------------------------------------------------------------------

inline StringId::StringId() : m_value(0) { m_string = "null"; }

inline StringId::StringId(Type val) : m_value(val) { m_string = "from value"; }

inline StringId::StringId(const char *string)
    : m_value(Math::crc /*< sizeof(string) >*/ (string)) {
  BASE_ASSERT(string, "null string not allowed");
  BASE_ASSERT(String::strlen(string) > 0, "empty string not allowed");
  // todo: RegisterString(*this, string);

  m_string = string;
}

inline bool StringId::IsValid() const { return *this != StringId::Invalid; }

inline bool StringId::operator==(const StringId &rhs) const {
  return m_value == rhs.m_value;
}

inline bool StringId::operator!=(const StringId &rhs) const {
  return m_value != rhs.m_value;
}

inline bool StringId::operator>(const StringId &rhs) const {
  return m_value > rhs.m_value;
}

inline bool StringId::operator<(const StringId &rhs) const {
  return m_value < rhs.m_value;
}

} // namespace Base
