/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"

namespace Base {

class RefCounted
{
public:
		inline RefCounted() : m_ref_count(0){}
		inline RefCounted( const RefCounted& rhs ) : m_ref_count(rhs.m_ref_count) { }
		inline RefCounted& operator=( const RefCounted& rhs ) { m_ref_count = rhs.m_ref_count; return *this; }
		
		inline void AddRefference() const { ++m_ref_count; }
		inline void RemRefference() const { --m_ref_count; }
		inline s32 UseCount() const { return m_ref_count; }
		inline bool Unique() const { return m_ref_count == 1; }
private:
		mutable s32 m_ref_count;
};

} // namespace Base
