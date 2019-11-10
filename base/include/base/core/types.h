/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

//---------------------------------------------------------------------------------------
// simple types.
//---------------------------------------------------------------------------------------

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long int s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
#if defined __ENGINE_WIN__
typedef unsigned __int64 u64;
#else
typedef unsigned long u64;
#endif
typedef char char8;
// typedef wchar_t					wchar16;

typedef s32 streamsize;
typedef float radian;
typedef float degree;
typedef u32 handle;
typedef s32 time_ms;

//---------------------------------------------------------------------------------------

enum Result {
  RS_SUCCESS = 0,
  RS_UNKNOWN = -1,
  RS_FAIL = -2,
  RS_INVALIDPARAM = -3,
  RS_IOFAIL = -4,
  RS_NOTENOUGH_MEM = -5,
  RS_ASYNC_IN_PROGRESS = -6,
  RS_BUFFER_TOO_SMALL = -7,
  RS_EMPTY = -8,
  RS_DISCONNECTED = -9
};

//---------------------------------------------------------------------------------------

#undef NULL
#define NULL 0

//---------------------------------------------------------------------------------------
