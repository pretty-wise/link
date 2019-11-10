/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/io/writer.h"

namespace Base {

class BinaryWriter : public Writer {
public:
  BinaryWriter(Stream &stream) : Writer(stream) {}

  bool WriteU8(u8 value);
  bool WriteS8(s8 value);
  bool WriteU16(u16 value);
  bool WriteS16(s16 value);
  bool WriteU32(u32 value);
  bool WriteS32(s32 value);
  bool WriteU64(u64 value);
  bool WriteS64(s64 value);
  bool WriteFloat(float value);
  bool WriteBool(bool value);
  bool WriteStringId(StringId value);
  bool WriteString(const char *);
};

} // namespace Base
