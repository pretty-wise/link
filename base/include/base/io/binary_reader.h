/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/io/reader.h"

namespace Base {

class BinaryReader : public Reader {
public:
  BinaryReader(Stream &stream) : Reader(stream) {}

  bool ReadU8(u8 *value);
  bool ReadS8(s8 *value);
  bool ReadU16(u16 *value);
  bool ReadS16(s16 *value);
  bool ReadU32(u32 *value);
  bool ReadS32(s32 *value);
  bool ReadU64(u64 *value);
  bool ReadS64(s64 *value);
  bool ReadFloat(float *value);
  bool ReadBool(bool *value);
  bool ReadStringId(StringId *value);
  bool ReadString(char *string, u32 &inout_length);
};

} // namespace Base
