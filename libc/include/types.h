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

#define cli() asm volatile("cli")
#define sti() asm volatile("sti")

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
	size_t fd;
} FILE;

typedef uint32_t mode_t;
typedef int32_t ssize_t;
typedef size_t off_t;