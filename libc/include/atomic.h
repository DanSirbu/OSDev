#pragma once
#include "sys/types.h"

//From musl
#define a_clz_64 a_clz_64
static inline int a_clz_64(uint64_t x)
{
	__asm__("bsr %1,%0 ; xor $63,%0" : "=r"(x) : "r"(x));
	return x;
}