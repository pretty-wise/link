/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"

#include <vector>

namespace Base {

class Handle {
public:
  static const u32 Invalid = (u32)-1;

  inline Handle() : m_handle(Invalid) {}
  inline Handle(u32 h) : m_handle(h) {}
  operator u32() { return m_handle; }

  bool operator==(const Handle &rhs) const { return m_handle == rhs.m_handle; }

protected:
  u32 m_handle;
};

class Handlable {
public:
  inline void SetHandle(Handle h) { m_handle = h; }

private:
  Handle m_handle;
};

template <class T> class ObjectArray {
public:
  ObjectArray(u32 uCapacity);
  ~ObjectArray();

public:
  inline bool Exists(Handle id) const;
  inline T &Get(Handle id);
  inline Handle Add();
  inline void Remove(Handle id);
  inline u32 Count() const;
  inline T &operator[](u32 index);
  inline bool IsFull() const { return Count() == m_uCapacity; }

private:
  struct Index {
    Handle id;
    u32 index;
    u32 next;
  };

  u32 m_uCapacity;
  u32 m_uCount;
  T *m_aObject;

  Index *m_aIndices;

  u32 m_uFirstFree;
  u32 m_uLastFree;
};

template <class T> ObjectArray<T>::ObjectArray(u32 uCapacity) {
  m_aObject = (T *)new s8[uCapacity * sizeof(T)];
  m_aIndices = new Index[uCapacity];

  m_uCapacity = uCapacity;
  m_uCount = 0;

  for(u32 i = 0; i < m_uCapacity; ++i) {
    m_aIndices[i].id = Handle(i);
    m_aIndices[i].next = i + 1;
  }

  m_uFirstFree = 0;
  m_uLastFree = uCapacity - 1;
}

template <class T> ObjectArray<T>::~ObjectArray() {
  delete[] m_aIndices;

  // Call remaining object's destructors.
  for(u32 i = 0; i < m_uCount; ++i)
    m_aObject[i].~T();

  // Delete object array memory
  delete[](s8 *)m_aObject;
}

template <class T> inline bool ObjectArray<T>::Exists(Handle id) const {
  return m_aIndices[(int)id].index < m_uCount;
}

template <class T> inline T &ObjectArray<T>::Get(Handle id) {
  BASE_ASSERT(Exists(id), "handle does not exists");
  return m_aObject[m_aIndices[(int)id].index];
}

template <class T> inline Handle ObjectArray<T>::Add() {
  BASE_ASSERT(m_uCount < m_uCapacity, "array full");

  // grab first free index data.
  Index &in = m_aIndices[m_uFirstFree];

  // remove index from free list.
  m_uFirstFree = in.next;

  // in.id += NEW_OBJECT_ID_ADD;

  // update object count and set index data.
  in.index = m_uCount++;

  // grab the object.
  T &o = m_aObject[in.index];

  // trigger constructor.
  new(&o) T();

  // and set its handle.
  // todo: do we need an object to have a handle? o.SetHandle(in.id);

  // return handle.
  return in.id;
}

template <class T> inline void ObjectArray<T>::Remove(Handle id) {
  // grab removed object index data.
  Index &in = m_aIndices[id];

  // grab remove object.
  T &o = m_aObject[in.index];

  // swap object with the last one in the array.
  o = m_aObject[--m_uCount];

  // trigger destructor
  m_aObject[m_uCount].~T();

  // update swapped object index entry.
  m_aIndices[o.GetHandle()].index = in.index;

  // mark removed handle index as invalid.
  in.index = (u32)-1;

  // add removed element to free list.
  m_aIndices[m_uLastFree].next = id;
  m_uLastFree = id;

  if(m_uFirstFree == m_uCapacity)
    m_uFirstFree = m_uLastFree;
}

template <class T> inline u32 ObjectArray<T>::Count() const { return m_uCount; }

template <class T> inline T &ObjectArray<T>::operator[](u32 index) {
  BASE_ASSERT(index < m_uCount, "index out of bounds");
  return m_aObject[index];
}

//---------------------------------------------------------------------------------------

template <class T> class RefCountedObjectArray : public ObjectArray<T> {
public:
  RefCountedObjectArray<T>(u32 capacity) : ObjectArray<T>(capacity) {}
  bool IsValid(Handle h) const { return ObjectArray<T>::Exists(h); }
  bool IsUnique(Handle h) const { return ObjectArray<T>::Get(h).IsUnique(); }
  void AddRefference(Handle h) { ObjectArray<T>::Get(h).AddRefference(); }
  void RemRefference(Handle h) { ObjectArray<T>::Get(h).RemRefference(); }
};

template <class T> class SharedHandle : public Handle {
public:
  SharedHandle(const SharedHandle &rhs) : m_owner(rhs.m_owner) {
    m_handle = rhs.m_handle;

    if(m_owner.IsValid(*this))
      m_owner.AddRefference(*this);
  }

  SharedHandle(RefCountedObjectArray<T> &owner, Handle h)
      : Handle(h), m_owner(owner) {
    if(m_owner.IsValid(*this))
      m_owner.AddRefference(*this);
  }

  virtual ~SharedHandle() {
    if(m_owner.IsValid(*this))
      m_owner.RemRefference(*this);
  }

  SharedHandle &operator=(const SharedHandle &r) {
    Reset(r.m_handle);
    return *this;
  }

  void Reset(Handle h) {
    if(m_handle != h) {
      if(m_owner.IsValid(h))
        m_owner.AddRefference(h);

      if(m_owner.IsValid(m_handle))
        m_owner.RemRefference(m_handle);

      m_handle = h;
    }
  }

  bool operator==(const SharedHandle &r) const { m_handle == m_handle; }
  bool operator!=(const SharedHandle &r) const { m_handle != m_handle; }

protected:
  RefCountedObjectArray<T> &m_owner;
};

} // namespace Base
