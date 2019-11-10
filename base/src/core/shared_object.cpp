/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/core/shared_object.h"
#include "base/core/assert.h"

#include <dlfcn.h>

namespace Base {

SharedObject::~SharedObject() { BASE_ASSERT(m_handle == NULL); }

bool SharedObject::Open(const char *path) {
  BASE_ASSERT(m_handle == NULL);
  m_handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
  if(!m_handle) {
    BASE_LOG_LINE("could not open so: %s. [%s]", path, dlerror());
  }
  return m_handle != NULL;
}

void SharedObject::Close() {
  if(m_handle) {
    if(0 == dlclose(m_handle)) {
      m_handle = NULL;
    }
  }
}

void *SharedObject::GetSymbol(const char *name) {
  BASE_ASSERT(m_handle);

  void *symbol = dlsym(m_handle, name);
  if(!symbol) {
    BASE_LOG_LINE("could not find symbol: %s. [%s]", name, dlerror());
  }

  return symbol;
}

} // namespace Base
