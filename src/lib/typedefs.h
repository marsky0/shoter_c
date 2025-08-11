#pragma once

#include <stdlib.h>
#include <string.h>

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

typedef enum EnumSystem {
    Windows,
    Unix
} EnumSystem;

#if defined(_WIN32) || defined(_WIN64)
    #define SYSTEM_PLATFORM Windows

    typedef unsigned char       u8;
    typedef unsigned short int  u16;
    typedef unsigned int        u32;
    typedef unsigned long long  u64;

    typedef char        i8;
    typedef short int   i16;
    typedef int         i32;
    typedef long long   i64;

    typedef float       f32;
    typedef double      f64;

#else
    #define SYSTEM_PLATFORM Unix

    #include <stdint.h>

    typedef uint8_t  u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;

    typedef int8_t  i8;
    typedef int16_t i16;
    typedef int32_t i32;
    typedef int64_t i64;

    typedef float f32;
    typedef double f64;

#endif


typedef struct {
	char* str;
	u64 length;
	u64 capacity;
} string;

void string_free(string* str);
void string_set(string* str, const char* chars);
string string_new(char* c);
string* string_split(string* str, char delim, u64* count);
void strings_sort(string* arr, u64 count);

#endif
