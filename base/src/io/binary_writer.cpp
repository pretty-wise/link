/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/io/binary_writer.h"
#include "base/io/stream.h"

#include "base/core/str.h"

namespace Base {

bool BinaryWriter::WriteU8(u8 value) { return m_stream.Write(&value, 1); }

bool BinaryWriter::WriteS8(s8 value) { return m_stream.Write(&value, 1); }

bool BinaryWriter::WriteU16(u16 value) { return m_stream.Write(&value, 2); }

bool BinaryWriter::WriteS16(s16 value) { return m_stream.Write(&value, 2); }

bool BinaryWriter::WriteU32(u32 value) { return m_stream.Write(&value, 4); }

bool BinaryWriter::WriteS32(s32 value) { return m_stream.Write(&value, 4); }

bool BinaryWriter::WriteU64(u64 value) { return m_stream.Write(&value, 8); }

bool BinaryWriter::WriteS64(s64 value) { return m_stream.Write(&value, 8); }

bool BinaryWriter::WriteFloat(float value) { return m_stream.Write(&value, 4); }

bool BinaryWriter::WriteBool(bool value) {
  u8 val = value ? 1 : 0;
  return m_stream.Write(&val, 1);
}

bool BinaryWriter::WriteStringId(StringId value) {
  BASE_ASSERT(sizeof(StringId::Type) == sizeof(u32));

  const StringId::Type val = value.GetValue();
  return m_stream.Write(&val, 4);
}

bool BinaryWriter::WriteString(const char *value) {
  if(!value)
    value = "";

  streamsize len = (streamsize)String::strlen(value);
  bool result = m_stream.Write(&len, 4);
  result |= m_stream.Write((void *)value, len + 1);
  return result;
}

} // namespace Base
