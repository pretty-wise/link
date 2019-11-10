/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"

#include <string> // size_t

namespace Base {

class Allocator {
public:
  enum { kDefaultAlignment = 4 };

  virtual ~Allocator() {}

  // Allocates specified bytes of memory using specified alignment.
  // @param bytes Number of bytes to allocate.
  // @param align Memory alignment.
  virtual void *Allocate(size_t bytes, size_t align = kDefaultAlignment) = 0;

  // Frees memory previously allocated by Allocate.
  // @param memory Memory to free.
  virtual void Deallocate(void *memory) = 0;
};

} // namespace Base
