/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/core/types.h"

namespace Base {
namespace String {

void FromString(const char* string, u8& value);
void FromString(const char* string, s8& value);
void FromString(const char* string, u16& value);
void FromString(const char* string, s16& value);
void FromString(const char* string, u32& value);
void FromString(const char* string, s32& value);
void FromString(const char* string, u64& value);
void FromString(const char* string, s64& value);

void ToString(u8 value, char* str, u32 length);
void ToString(s8 value, char* str, u32 length);
void ToString(u16 value, char* str, u32 length);
void ToString(s16 value, char* str, u32 length);
void ToString(u32 value, char* str, u32 length);
void ToString(s32 value, char* str, u32 length);
void ToString(u64 value, char* str, u32 length);
void ToString(s64 value, char* str, u32 length);
void ToString(float value, char* str, u32 length);

int CompareNoCase(const char* a, const char* b, u32 lenght);
int CompareN(const char* a, const char* b, u32 lenght);
int Compare(const char* a, const char* b);

int strlen(const char*);

void strncpy(char* dest, const char* src, u32 n);

} // namespace String
} // namespace Base
