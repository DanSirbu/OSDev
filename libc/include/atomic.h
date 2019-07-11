#pragma once
#include "sys/types.h"

#define a_clz_32 a_clz_32
static inline int a_clz_32(uint32_t x)
{
	__asm__("bsr %1,%0 ; xor $31,%0" : "=r"(x) : "r"(x));
	return x;
}

static inline int a_clz_64(uint64_t x)
{
	if (x >> 32)
		return a_clz_32(x >> 32);
	return a_clz_32(x) + 32;
}