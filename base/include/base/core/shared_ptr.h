/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"

namespace Base {

template<class T>
class SharedPtr
{
public:
		SharedPtr(T* p = NULL);
		SharedPtr(const SharedPtr& r);
		SharedPtr();
		
		T& operator*() const;
		T* operator->() const;
		operator T*() const;
		T* get() const;
		
		SharedPtr& operator=(T* p);
		SharedPtr& operator=(const SharedPtr& r);
		
		bool operator==(T* p) const;
		bool operator!=(T* p) const;
		bool operator==(const SharedPtr& r) const;
		bool operator!=(const SharedPtr& r) const;
		
		void reset();
		void reset(T* p);
		void reset(const SharedPtr& r, T* p);
		
		
		bool unique() const;
		s32	use_count() const;
		bool is_valid() const;
		
		
		void swap(shared_ptr& b);
		
private:
		T* m_pObject;
};


template<class T>
SharedPtr<T>::SharedPtr(T* p)
{
	m_pObject = p;
	if (m_pObject)
		m_pObject->addReference();
}

template<class T>
SharedPtr<T>::SharedPtr(const shared_ptr& r)
{
	m_pObject = r.m_pObject;
	if (m_pObject)
		m_pObject->addReference();
}

template<class T>
SharedPtr<T>::SharedPtr()
{
	if (m_pObject)
		m_pObject->releaseReference();
}

template<class T>
T& SharedPtr<T>::operator*() const
{
	return *static_cast<T*>(m_pObject);
}

template<class T>
T* SharedPtr<T>::operator->() const
{
	return static_cast<T*>(m_pObject);
}

template<class T>
SharedPtr<T>::operator T*() const
{
	return static_cast<T*>(m_pObject);
}

template<class T>
T* SharedPtr<T>::get() const
{
	return static_cast<T*>(m_pObject);;
}

template<class T>
SharedPtr<T>& SharedPtr<T>::operator=(T* p)
{
	reset(p);
	return *this;
}

template<class T>
shared_ptr<T>& SharedPtr<T>::operator=(const shared_ptr<T>& r)
{
	reset(r.m_pObject);
	return *this;
}

template<class T>
bool SharedPtr<T>::operator==(T* p) const
{
	return m_pObject == p;
}

template<class T>
bool SharedPtr<T>::operator!=(T* p) const
{
	return m_pObject != p;
}

template<class T>
bool SharedPtr<T>::operator==(const SharedPtr<T>& r) const
{
	return m_pObject == r.m_pObject;
}

template<class T>
bool SharedPtr<T>::operator!=(const SharedPtr<T>& r) const
{
	return m_pObject != r.m_pObject;
}

template<class T>
void SharedPtr<T>::reset()
{
	if (m_pObject)
		m_pObject->releaseReference();
		
	m_pObject = NULL;
}

template<class T>
void SharedPtr<T>::reset(T* p)
{
	if (m_pObject != p)
	{
		if (p)
			p->addReference();
				
		if (m_pObject)
			m_pObject->releaseReference();
				
		m_pObject = p;
	}
}

template<class T>
void SharedPtr<T>::reset(const shared_ptr<T>& r, T* p)
{
}

template<class T>
bool SharedPtr<T>::unique() const
{
	return use_count() == 1;
}

template<class T>
s32 SharedPtr<T>::use_count() const
{
	if (m_pObject)
		return m_pObject->getNumReferences();
	else
		return 0;
}

template<class T>
bool SharedPtr<T>::is_valid() const
{
	return (m_pObject != 0);
}

template<class T>
void SharedPtr<T>::swap(shared_ptr<T>& b)
{
	reset (b.m_pObject);
}

} // namespace Base
