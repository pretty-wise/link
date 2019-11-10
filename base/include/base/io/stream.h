/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"

namespace Base {

class Stream
{
public:
		inline Stream() : m_position(0), m_length(0){}
		
		virtual bool Open( s8* pointer, const streamsize length ) = 0;
		virtual bool Close( void ) = 0;
		
		virtual bool Seek( s32 relativeOffset ) = 0;
		virtual bool SeekBeg( streamsize offset = 0 ) = 0;
		virtual bool SeekEnd( streamsize offset = 0 ) = 0;
		
		virtual bool Write( const void* dataPtr, const streamsize len ) = 0;
		virtual bool Read( void* dataPtr, const streamsize len ) = 0;
		
		inline streamsize GetPosition( void ) const { return m_position; }
		inline streamsize GetLength( void ) const { return m_length; }
		inline bool IsEOF( void ) const { return m_position == m_length; }
		
protected:
		streamsize m_position;
		streamsize m_length;
};

} // namespace Base
