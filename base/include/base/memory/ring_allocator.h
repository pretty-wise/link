/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/memory/allocator.h"

namespace Base {

/* Ring Allocator, circular allocator or cyclic allocator class.
 * Allocates memory from a specified buffer in a cyclic manner.
 * During allocation, the head pointer is being moved forward such as it does
 * not touch the tail pointer.
 * When the allocation goes beyond the end of the buffer, it tries to allocate
 * from the beginning of the buffer and marks the remaining part as deallocated.
 * During deallocation, freed memory is being marked as deallocated and tail
 * pointer
 * gets moved as much forward as possible.
 * If tail pointer meets head pointer during deallocation than both are moved to
 * the beginning of the buffer.
 * The 32 bit int is used to store both the size of the allocation and the
 * status
 * of the allocation. That means a total size of single allocation cannot exceed
 * 2147483647 bytes.
 */
class RingAllocator : public Allocator {
public:
  RingAllocator();
  /// C-tor.
  /// The allocator itself does not manage the memory it allocates from.
  /// @param memory A buffer to allocate from.
  /// @param size The size of provided buffer
  RingAllocator(void *memory, unsigned int size);

  /// D-tor.
  ~RingAllocator();

  /// Allocates specified bytes of memory using specified alignment.
  /// The size of allocated block is rounded up to multiple of 4.
  /// @param bytes Number of bytes to allocate.
  /// @param align Memory alignment.
  void *Allocate(size_t bytes, size_t align = Allocator::kDefaultAlignment);

  /// Frees memory previously allocated by Allocate.
  /// @param memory Memory to free.
  void Deallocate(void *memory);

  /// Returns the size of previously allocated block.
  /// The size of allocation may be different from the requested size - it's
  /// rounded up to multiple of 4.
  /// @param allocation Allocation to check the size of.
  unsigned int Size(void *allocation) const;

  /// Returns true if there are no allocations, false otherwise.
  bool Empty() const { return m_head == m_tail; }

  unsigned int Capacity() const { return m_end - m_begin; }

  /// Clears all allocations.
  void Clear() {
    m_head = m_begin;
    m_tail = m_begin;
  }

private:
  char *m_begin, *m_end; //< buffer boundaries.
  char *m_head, *m_tail; //< current allocation, deallocation points.
};

} // namespace Base
