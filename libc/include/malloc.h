#pragma once
#include "types.h"

void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

void *memset(void *ptr, int value, size_t s);
void *memcpy(void *dst, const void *src, size_t s);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);