#pragma once
#include "types.h"

#define KERN_BASE 0xC0000000

void *kmalloc(size_t size);
void *kmalloc_align(size_t size, uint8_t alignment);
void kfree(void *ptr);
void *sbrk(size_t size);

void memset(void *ptr, char value, size_t s);
void memcpy(void *dst, void *src, size_t s);
