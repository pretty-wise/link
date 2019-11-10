/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include "base/core/string_id.h"

namespace Base {

class Stream;

class Writer
{
public:
		Writer( Stream& stream ) : m_stream(stream){}
		
		virtual bool WriteU8( u8 value ) = 0;
		virtual bool WriteS8( s8 value ) = 0;
		virtual bool WriteU16( u16 value ) = 0;
		virtual bool WriteS16( s16 value ) = 0;
		virtual bool WriteU32( u32 value ) = 0;
		virtual bool WriteS32( s32 value ) = 0;
		virtual bool WriteU64( u64 value ) = 0;
		virtual bool WriteS64( s64 value ) = 0;
		virtual bool WriteFloat( float value ) = 0;
		virtual bool WriteBool( bool value ) = 0;
		virtual bool WriteStringId( StringId value ) = 0;
		virtual bool WriteString( const char* ) = 0;
		
protected:
		Stream& m_stream;
};

} // namespace Base
