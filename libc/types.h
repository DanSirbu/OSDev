#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
// typedef unsigned long long int u64;

typedef u8 uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
// typedef u64 uint64_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;

typedef u32 size_t;

typedef size_t vptr_t; //Pointer to virtual memory
typedef size_t pptr_t; //Pointer to physical memory

#define NULL 0

#define false 0
#define true 1

#define FALSE 0
#define TRUE 1

typedef uint8_t bool;

#define ALIGN(val, alignment)                                                  \
	if (val % alignment != 0) {                                            \
		val += alignment - (val % alignment);                          \
	}
