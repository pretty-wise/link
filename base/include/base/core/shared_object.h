/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"

namespace Base {

class SharedObject {
public:
  SharedObject() : m_handle(nullptr) {}
  ~SharedObject();

  bool Open(const char *path);
  void Close();
  bool IsLoaded() const { return m_handle != nullptr; }
  void *GetSymbol(const char *symbol);

private:
  void *m_handle;
};

} // namespace Base
