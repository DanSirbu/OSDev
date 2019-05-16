#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

typedef size_t vptr_t; //Pointer to virtual memory
typedef size_t pptr_t; //Pointer to physical memory

#define ALIGN(val, alignment)                                                  \
	if (val % alignment != 0) {                                            \
		val += alignment - (val % alignment);                          \
	}

#define cli() asm volatile("cli")
#define sti() asm volatile("sti")