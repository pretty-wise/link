/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/memory/ring_buffer.h"
#include "base/core/assert.h"
#include "base/memory/memory.h"

namespace Base {

RingBuffer::RingBuffer(void *memory, u32 numBytes)
    : m_numBytes(numBytes), m_readIndex(0), m_writeIndex(0),
      m_readBytesAvailable(0), m_writeBytesAvailable(0),
      m_memory(static_cast<u8 *>(memory)) {}

u32 RingBuffer::Write(const void *src, u32 numBytes) {
  if(numBytes == 0) {
    return 0;
  }
  if(numBytes > m_writeBytesAvailable) {
    return 0; // we could clamp, but writing partial data is useless.
  }
  u32 remainingBytes = m_numBytes - m_writeIndex;
  if(numBytes > remainingBytes) {
    memcpy(m_memory + m_writeIndex, src, remainingBytes);
    memcpy(m_memory, (u8 *)src + remainingBytes, numBytes - remainingBytes);
  } else {
    memcpy(m_memory + m_writeIndex, src, numBytes);
  }
  m_writeIndex = (m_writeIndex + numBytes) % m_numBytes;
  m_writeBytesAvailable -= numBytes;
  m_readBytesAvailable += numBytes;
  return numBytes;
}

u32 RingBuffer::Read(void *dst, u32 numBytes) {
  BASE_ASSERT(dst);
  if(numBytes == 0) {
    return 0;
  }
  if(numBytes > m_readBytesAvailable) {
    return 0; // we could clamp, but reading partial data is useless.
  }
  u32 remainingBytes = m_numBytes - m_readIndex;
  if(numBytes > remainingBytes) {
    memcpy(dst, m_memory + m_readIndex, remainingBytes);
    memcpy((u8 *)dst + remainingBytes, m_memory, numBytes - remainingBytes);
  } else {
    memcpy(dst, m_memory + m_readIndex, numBytes);
  }
  m_readIndex = (m_readIndex + numBytes) % m_numBytes;
  m_writeBytesAvailable += numBytes;
  m_readBytesAvailable -= numBytes;
  return numBytes;
}

} // namespace Base
