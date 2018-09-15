#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long int u64;


typedef u8 uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;

typedef u32 size_t;

#define NULL 0
#define false 0

#define ALIGN(val, alignment) \
    if(val % alignment != 0) { \
        val += alignment - (val % alignment); \
    }