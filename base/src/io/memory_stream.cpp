/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/io/memory_stream.h"
#include "base/core/assert.h"

#include <vector>
#include <string.h>

namespace Base {

MemoryStream::MemoryStream() {}

MemoryStream::~MemoryStream() {}

bool MemoryStream::Open(s8 *pointer, const streamsize length) {
  BASE_ASSERT(pointer);

  m_buffer = pointer;
  m_length = length;

  // init
  m_position = 0;

  return true;
}

bool MemoryStream::Close(void) {
  m_buffer = nullptr;
  m_position = 0;
  m_length = 0;

  return true;
}

bool MemoryStream::Seek(s32 relativeOffset) {

  if(relativeOffset < 0 && m_position + relativeOffset < 0) {
    BASE_LOG("cannot seek back that much.\n");
    return false;
  }

  streamsize new_pos = m_position + relativeOffset;

  if(new_pos > m_length) {
    BASE_LOG("cannot seek that far\n");
    return false;
  }

  m_position = new_pos;

  return true;
}

bool MemoryStream::SeekBeg(streamsize offset) {
  BASE_ASSERT(false);

  return true;
}

bool MemoryStream::SeekEnd(streamsize offset) {
  BASE_ASSERT(false);

  return true;
}

bool MemoryStream::Write(const void *dataPtr, const streamsize len) {
  if(m_position + len > m_length)
    return false;

  memcpy(&m_buffer[(size_t)m_position], dataPtr, (size_t)len);
  m_position += len;

  return true;
}
bool MemoryStream::Read(void *dataPtr, const streamsize len) {
  BASE_ASSERT(dataPtr);

  if(m_position + len > m_length) {
    BASE_LOG("out of stream\n");
    return false;
  }

  memcpy(dataPtr, &m_buffer[m_position], (size_t)len);
  m_position += len;

  return true;
}

} // namespace Base
