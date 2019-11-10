/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/io/bit_reader.h"
#include "base/io/stream.h"
#include "base/core/macro.h"

namespace Base {

bool BitReader::ReadU8(u8 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 1);
}

bool BitReader::ReadS8(s8 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 1);
}

bool BitReader::ReadU16(u16 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 2);
}

bool BitReader::ReadS16(s16 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 2);
}

bool BitReader::ReadU32(u32 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 4);
}

bool BitReader::ReadS32(s32 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 4);
}

bool BitReader::ReadU64(u64 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 8);
}

bool BitReader::ReadS64(s64 *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 8);
}

bool BitReader::ReadFloat(float *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");
  return m_stream.Read(value, 4);
}

bool BitReader::ReadBool(bool *value) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");

  u8 res;
  bool result = m_stream.Read(&res, 1);
  *value = res != 0;
  return result;
}

bool BitReader::ReadStringId(StringId *value) {
  BASE_ASSERT(sizeof(StringId::Type) == sizeof(u32));

  u32 val;
  bool result = m_stream.Read(&val, 4);
  *value = StringId(val);
  return result;
}

bool BitReader::ReadString(char *string, u32 &inout_length) {
  BASE_ASSERT(!m_stream.IsEOF(), "EOF Reached");

  if(!string) {
    // check the length only
    return m_stream.Read(&inout_length, 4);
  }
  return m_stream.Read(string, inout_length);
}

} // namespace Base
