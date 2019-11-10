/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/io/binary_reader.h"
#include "base/io/stream.h"
#include "base/core/macro.h"

namespace Base {

bool BinaryReader::ReadU8(u8 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 1);
}

bool BinaryReader::ReadS8(s8 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 1);
}

bool BinaryReader::ReadU16(u16 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 2);
}

bool BinaryReader::ReadS16(s16 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 2);
}

bool BinaryReader::ReadU32(u32 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 4);
}

bool BinaryReader::ReadS32(s32 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 4);
}

bool BinaryReader::ReadU64(u64 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 8);
}

bool BinaryReader::ReadS64(s64 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 8);
}

bool BinaryReader::ReadFloat(float *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 4);
}

bool BinaryReader::ReadBool(bool *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");

  u8 res;
  bool result = m_stream.Read(&res, 1);
  *value = res != 0;
  return result;
}

bool BinaryReader::ReadStringId(StringId *value) {
  BASE_ASSERT(sizeof(StringId::Type) == sizeof(u32));

  u32 val;
  bool result = m_stream.Read(&val, 4);
  *value = StringId(val);
  return result;
}

bool BinaryReader::ReadString(char *string, u32 &inout_length) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");

  if(!string) {
    // check the length only
    return m_stream.Read(&inout_length, 4);
  }
  return m_stream.Read(string, inout_length);
}

} // namespace Base
