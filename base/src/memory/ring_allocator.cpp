/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/memory/ring_allocator.h"
#include "base/core/assert.h"

namespace Base {

// Bit that marks if memory is not used.
static const unsigned int kAllocationStatusBit = 0x80000000u;
// Bits that store size of the allocation.
static const unsigned int kAllocationSizeBits = 0x7fffffffu;

// Memory header. Followed by allocated memory.
struct Header {
  // Number of bytes taken by Header, padding and allocated data.
  unsigned int size;
  // Allocation buffer comes after Header.
  char data[0];
};

/// Returns pointer to data block given a pointer to a valid allocation header.
char *GetData(Header *header, size_t alignment) {
  // TODO(kstasik) resolve alignment.
  (void)&alignment;
  return header->data;
}

// Returns pointer to allocation header given a pointer to the allocated data.
Header *GetHeader(void *data) {
  // TODO(kstasik) resolve padding.
  return reinterpret_cast<Header *>((char *)data - sizeof(Header));
}

RingAllocator::RingAllocator()
    : m_begin(nullptr), m_end(nullptr), m_head(nullptr), m_tail(nullptr) {}

RingAllocator::RingAllocator(void *memory, unsigned int size)
    : m_begin((char *)memory),
      // round down memory used to multiple of Header size.
      m_end((char *)memory + ((size / sizeof(Header)) * sizeof(Header))),
      m_head(m_begin), m_tail(m_begin) {}

RingAllocator::~RingAllocator() { BASE_ASSERT(Empty()); }

void *RingAllocator::Allocate(size_t bytes_, size_t align) {
  BASE_ASSERT(0 == align % 4);
  BASE_ASSERT(bytes_ <= kAllocationSizeBits);
  BASE_ASSERT((char *)m_head + sizeof(Header) <= m_end);

  // for now, alignment is fixed at 4 bytes.
  BASE_ASSERT(Allocator::kDefaultAlignment == align &&
              Allocator::kDefaultAlignment == 4);

  // round-up the size of the buffer to multiple of Header size.
  // this way Header always fits at the end of the buffer if m_head < m_end.
  unsigned int bytes =
      ((static_cast<unsigned int>(bytes_) + sizeof(Header) - 1) /
       sizeof(Header)) *
      sizeof(Header);

  Header *header = reinterpret_cast<Header *>((char *)m_head);
  char *data_begin = GetData(header, align);
  char *data_end = data_begin + bytes;

  if(data_end < m_end) {
    // data fits before buffer end.

    // m_tail == m_head - ok, buffer empty.
    // m_tail < m_head - ok, because data_end < m_end
    // m_tail > m_head - need to check..

    if(m_tail > m_head && data_end >= m_tail) {
      // we go beyond m_tail here.
      return NULL;
    }
  } else {
    // buffer overflow, we need to wrap to m_begin
    // we can do that only if head is greater then tail
    if(m_head <= m_tail) {
      return NULL;
    }

    // this is the last possible chunk in the buffer - let's mark it
    // deallocated.
    header->size = (unsigned int)((m_end - m_head) | kAllocationStatusBit);

    // try fitting from start of the buffer.
    header = reinterpret_cast<Header *>((char *)m_begin);
    data_begin = GetData(header, align);
    data_end = data_begin + bytes;

    // check if there is enough space before m_tail
    if(data_end >= m_tail) {
      return NULL;
    }
  }

  m_head = data_end;                     // move head pointer.
  header->size = bytes + sizeof(Header); // marks as allocated
  return data_begin;
}

void RingAllocator::Deallocate(void *memory) {
  if(!memory) {
    return; // NULL delete.
  }

  BASE_ASSERT(memory > m_begin && memory < m_end,
              "memory out of allocation boundaries.");

  Header *header = GetHeader(memory);

  BASE_ASSERT((header->size & kAllocationStatusBit) == 0,
              "memory not allocated");

  // mark as deallocated.
  header->size |= kAllocationStatusBit;

  // advance m_tail as much as possible.
  while(m_tail != m_head) {
    BASE_ASSERT(m_tail < m_end);
    Header *header = reinterpret_cast<Header *>(m_tail);
    if((header->size & kAllocationStatusBit) == 0) {
      // memory used.
      break;
    }

    // block is empty - advance m_tail.
    m_tail += header->size & kAllocationSizeBits;

    // m_end reached, start from m_begin.
    if(m_tail == m_end) {
      m_tail = m_begin;
    }
  }

  if(m_head == m_tail) {
    // the buffer is empty so we move pointers to m_begin.
    m_head = m_begin;
    m_tail = m_begin;
  }
}

unsigned int RingAllocator::Size(void *allocation) const {
  Header *header = GetHeader(allocation);
  // TODO: resolve padding.
  return header->size - sizeof(Header);
}

} // namespace Base
