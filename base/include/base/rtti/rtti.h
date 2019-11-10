/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include "base/core/string_id.h"
#include "rtti/rtti.h"

#include <vector>

namespace Base {

// template to define type of identifier. matters for the size of networked
// messages.

class Rtti {
public:
  const Rtti(const Rtti *base_type, StringId type)
      : m_base_type(base_type), m_type(type) {}

  inline bool IsExactly(const Rtti &type) const { return &type == this; }

  inline bool IsDerived(const Rtti &type) const {
    const Rtti *search = this;
    while(search) {
      if(search == &type)
        return true;
      search = search->m_base_type;
    }
    return false;
  }

  inline StringId GetTypeName() const { return m_type; }

private:
  const Rtti *m_base_type;
  StringId m_type;
};

#define DECLARE_RTTI                                                           \
public:                                                                        \
  static const Base::Rtti TYPE;                                                \
  virtual const Base::Rtti &GetType() const { return TYPE; }

#define DEFINE_RTTI(classname, baseclassname)                                  \
  const Base::Rtti classname::TYPE(&baseclassname::TYPE,                       \
                                   Base::StringId(#classname))

#define DEFINE_RTTI_BASE(classname)                                            \
  const Base::Rtti classname::TYPE(nullptr, Base::StringId(#classname))

} // namespace Base
