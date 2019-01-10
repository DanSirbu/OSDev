#pragma once
#include "types.h"
#include "mmu.h"

void *kmalloc(size_t size);
void *kcalloc(size_t size);
void *kmalloc_align(size_t size, uint8_t alignment);
void *kvmalloc(size_t size);

void kfree(void *ptr);
void *sbrk(size_t size);
void kinit_malloc(vptr_t start, vptr_t end);

void memset(void *ptr, char value, size_t s);
void memcpy(void *dst, void *src, size_t s);
