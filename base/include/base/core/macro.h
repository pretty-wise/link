/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/log.h"
#include <stdio.h>

#define BASE_FILENAME __FILE__
#define BASE_LINE __LINE__
#define BASE_FUNCTION __PRETTY_FUNCTION__

#define BASE_LIB(libName) __pragma(comment(lib, libName));

#define NO_COPY(classname)                                                     \
  classname(const classname &rhs);                                             \
  classname &operator=(const classname &)

// macro returns the number of arguments that have been passed to it. works with
// __VA_ARGS__.

#define BASE_NARG(...) _BASE_NARG(__VA_ARGS__, _BASE_RSEQ_N())
#define _BASE_NARG(...) _BASE_ARG_N(__VA_ARGS__)
#define _BASE_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
#define _BASE_RSEQ_N() 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

// joins two preprocessor strings.
#define BASE_JOIN(a, b) _BASE_JOIN(a, b)
#define _BASE_JOIN(a, b) __BASE_JOIN(a, b)
#define __BASE_JOIN(a, b) a##b

#if defined _DEBUG || defined DEBUG
#define BASE_DEBUG_BUILD 1
#define BASE_LOG printf
#define BASE_LOG_LINE(...)                                                     \
  BASE_LOG(__VA_ARGS__);                                                       \
  BASE_LOG("\n")
#else
#define BASE_FINAL_BUILD 1
#define BASE_LOG(...)
#define BASE_LOG_LINE(...)
#endif
