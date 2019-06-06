#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>

typedef size_t vptr_t; //Pointer to virtual memory
typedef size_t pptr_t; //Pointer to physical memory

#define ALIGN(val, alignment)                                                  \
	if (val % alignment != 0) {                                            \
		val += alignment - (val % alignment);                          \
	}

#define cli() __asm__ volatile("cli")
#define sti() __asm__ volatile("sti")

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))