#pragma once
#include "types.h"

void *malloc(size_t size);
void free(void *ptr);

void memset(void *ptr, char value, size_t s);
void memcpy(void *dst, void *src, size_t s);