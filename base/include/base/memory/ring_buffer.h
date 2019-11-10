/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include "base/core/types.h"

namespace Base {

class RingBuffer {
public:
  RingBuffer(void *memory, u32 numBytes);
  u32 Write(const void *src, u32 numBytes);
  u32 Read(void *dst, u32 numBytes);

private:
  u32 m_numBytes;
  u32 m_readIndex;
  u32 m_writeIndex;
  u32 m_readBytesAvailable;
  u32 m_writeBytesAvailable;
  u8 *m_memory;
};

} // namespace Base
