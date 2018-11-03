#pragma once
#include "types.h"

#define KERN_BASE 0xC0000000

void *kmalloc(size_t size);
void *kmalloc_align(size_t size, uint8_t alignment);
void kfree(void *ptr);
void *sbrk(size_t size);

void memset(char *ptr, uint8_t value, size_t s);
void memcpy(char *dst, char *src, size_t s);