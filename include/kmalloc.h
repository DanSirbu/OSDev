#pragma once
#include "sys/types.h"
#include "mmu.h"

void *kmalloc(size_t size);
void *kcalloc(size_t size);
void *kmalloc_align(size_t size, uint8_t alignment);
void *kvmalloc(size_t size);
void *krealloc(void *ptr, size_t newSize);

void kfree(void *ptr);
void kfree_arr(char **ptr);

void *sbrk(ssize_t size);
void kinit_malloc(size_t start, size_t end);

void memset(void *ptr, char value, size_t s);
void memcpy(void *dst, void *src, size_t s);
