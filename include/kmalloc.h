#pragma once

void* kmalloc(size_t size);
void kfree(void *ptr);
void *sbrk(size_t size);