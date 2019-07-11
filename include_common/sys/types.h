#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include "features.h"

//#define __cplusplus //So boolean isn't redefined

#define ALIGN(val, alignment)                                                  \
	if (val % alignment != 0) {                                            \
		val += alignment - (val % alignment);                          \
	}

#define cli() __asm__ volatile("cli")
#define sti() __asm__ volatile("sti")

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
	size_t fd;
} FILE;

typedef int pid_t;
typedef uint32_t mode_t;
typedef int32_t ssize_t;
typedef size_t off_t;

//For some reason in math.h, this is defined later than it is used so we must define it here
typedef float float_t;
typedef double double_t;

#define EOF (-1)