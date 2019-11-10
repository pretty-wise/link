/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/core/str.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace Base {
namespace String {

void FromString(const char* string, u8& value) {
	value = (u8)atoi(string);
}

void FromString(const char* string, s8& value) {
	value = (s8)atoi(string);
}

void FromString(const char* string, u16& value) {
	value = (u16)atoi(string);
}

void FromString(const char* string, s16& value) {
	value = (s16)atoi(string);
}

void FromString(const char* string, u32& value) {
	value = (u32)atoi(string);
}

void FromString(const char* string, s32& value) {
	value = (s32)atoi(string);
}

void FromString(const char* string, u64& value) {
	value = (u64)atol(string);
}

void FromString(const char* string, s64& value) {
	value = (s64)atol(string);
}

void ToString(u8 value, char* str, u32 length) {
	snprintf(str, length, "%d", value);
}

void ToString(s8 value, char* str, u32 length) {
	snprintf(str, length, "%d", value);
}

void ToString(u16 value, char* str, u32 length) {
	snprintf(str, length, "%d", value);
}

void ToString(s16 value, char* str, u32 length) {
	snprintf(str, length, "%d", value);
}

void ToString(u32 value, char* str, u32 length) {
	snprintf(str, length, "%d", value);
}

void ToString(s32 value, char* str, u32 length) {
	snprintf(str, length, "%d", value);
}

void ToString(u64 value, char* str, u32 length) {
	snprintf(str, length, "%lu", value);
}

void ToString(s64 value, char* str, u32 length) {
	snprintf(str, length, "%lld", value);
}

void ToString(float value, char* str, u32 length) {
	snprintf(str, length, "%f", value);
}

int CompareNoCase(const char* a, const char* b, u32 length) {
	// todo: lowercase the strings.
	return ::strncmp(a, b, length);
}

int CompareN(const char* a, const char* b, u32 length) {
	return ::strncmp(a, b, length);
}

int Compare(const char* a, const char* b) {
	return ::strcmp(a, b);
}

int strlen(const char* str) {
	return ::strlen(str);
}

void strncpy(char* dest, const char* src, u32 n) {
	::strncpy(dest, src, n);
}

} // namespace String
} // namespace Base
