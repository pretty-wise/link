/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include <assert.h>
#include "base/core/macro.h"

#if defined(_DEBUG) || defined(DEBUG)
#define _BASE_USE_ASSERT
#endif

#if defined(_BASE_USE_ASSERT)
#define __ASSERT assert(false);
#define _BASE_ASSERT_0(exp, ...)                                               \
  if(!(exp)) {                                                                 \
    BASE_LOG("[ASSERT] in \"%s\":%d. (%s) == FALSE. ", __FILE__, __LINE__,     \
             #exp);                                                            \
    BASE_LOG(__VA_ARGS__);                                                     \
    BASE_LOG(".\n");                                                           \
    __ASSERT                                                                   \
  }
#define _BASE_ASSERT_1(exp)                                                    \
  if(!(exp)) {                                                                 \
    BASE_LOG("[ASSERT] in \"%s\":%d. (%s) == FALSE.\n", __FILE__, __LINE__,    \
             #exp);                                                            \
    __ASSERT                                                                   \
  }
#else // _BASE_USE_ASSERT
#define _BASE_ASSERT_0(exp, ...) (void)(exp)
#define _BASE_ASSERT_1(exp) (void)(exp)
#endif // _BASE_USE_ASSERT

#define _ASSERT_RESOLVE(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _9
#define __ASSERT_RESOLVE(...)                                                  \
  _ASSERT_RESOLVE(__VA_ARGS__, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0)
#define ___ASSERT_RESOLVE(...)                                                 \
  BASE_JOIN(_BASE_ASSERT_, __ASSERT_RESOLVE(__VA_ARGS__))(__VA_ARGS__)

#define BASE_ASSERT(...) ___ASSERT_RESOLVE(__VA_ARGS__)
