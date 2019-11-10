/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include "base/core/string_id.h"

namespace Base {

class Stream;

class Reader
{
public:
		Reader( Stream& stream ) : m_stream(stream) {}
		
		virtual bool ReadU8(u8* value) = 0;
		virtual bool ReadS8(s8* value) = 0;
		virtual bool ReadU16(u16* value) = 0;
		virtual bool ReadS16(s16* value) = 0;
		virtual bool ReadU32(u32* value) = 0;
		virtual bool ReadS32(s32* value) = 0;
		virtual bool ReadU64(u64* value) = 0;
		virtual bool ReadS64(s64* value) = 0;
		virtual bool ReadFloat(float* value) = 0;
		virtual bool ReadBool(bool* value) = 0;
		virtual bool ReadStringId(StringId* value) = 0;
		virtual bool ReadString( char* string, u32& inout_length ) = 0;
		
		bool IsEOF() const;
		
protected:
		Stream& m_stream;
};

} // namespace Base
