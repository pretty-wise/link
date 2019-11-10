/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include "rtti/rtti.h"

#include <vector>

namespace Base {

template <class BaseType> class BaseTypeCreator {
public:
  virtual ~BaseTypeCreator() {}
  virtual BaseType *operator()() { return nullptr; }
  virtual BaseType *placement_new(s8 *mem, size_t mem_size) { return nullptr; }
};

template <class T, class BaseType>
class TypeCreator : public BaseTypeCreator<BaseType> {
public:
  BaseType *operator()() { return new T(); }

  BaseType *placement_new(s8 *mem, size_t mem_size) {
    BASE_ASSERT(mem_size == sizeof(T));
    return new(mem) T();
  }
};

template <class BaseType> class DynamicTypes {
public:
  ~DynamicTypes() {
    // todo:
    // for( std::vector<class_data>::iterator it = m_indices.begin(); it !=
    // m_indices.end(); ++it )
    {
      //		delete (*it).creator;
    }
  }
  template <class T> void RegisterType() {
    const Rtti &rtti = T::TYPE;

    class_data cdata;
    cdata.type_name = rtti.GetTypeName();
    cdata.type_size = sizeof(T);
    cdata.creator = new TypeCreator<T, BaseType>();

    m_indices.push_back(cdata);
  }

  u32 TypeIndex(StringId type_name) {
    for(u32 i = 0; i < m_indices.size(); ++i) {
      if(m_indices[i].type_name == type_name)
        return i;
    }

    BASE_ASSERT(false, "unknown message %s", type_name.c_str());
    return 0;
  }

  size_t TypeSize(u32 index) {
    BASE_ASSERT(index < m_indices.size(), "index out of scope");
    return m_indices[index].type_size;
  }

  StringId TypeName(u32 index) {
    BASE_ASSERT(index < m_indices.size(), "index out of scope");
    return m_indices[index].type_name;
  }

  BaseType *Create(u32 index) { return (*m_indices[index].creator)(); }

  BaseType *Create(u32 index, void *mem, size_t mem_size) {
    m_indices[index].creator->placement_new(mem, mem_size);
  }

  u32 Count() const { return m_indices.size(); }

private:
  struct class_data {
    StringId type_name;
    size_t type_size;
    int creator_index;
    BaseTypeCreator<BaseType> *creator;
  };

  std::vector<class_data> m_indices;
};

} // namespace Base
