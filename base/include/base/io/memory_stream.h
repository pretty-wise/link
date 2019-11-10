/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/io/stream.h"

namespace Base {

class MemoryStream : public Stream
{
public:
		MemoryStream();
		~MemoryStream();
		
		bool Open( s8* pointer, const streamsize length );
		bool Close( void );
		
		bool Seek( s32 relativeOffset );
		bool SeekBeg( streamsize offset = 0 );
		bool SeekEnd( streamsize offset = 0 );
		
		bool Write( const void* dataPtr, const streamsize len );
		bool Read( void* dataPtr, const streamsize len );
private:
		s8* m_buffer;
};

} // namespace Base
