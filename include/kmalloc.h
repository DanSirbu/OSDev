#pragma once
#define KERN_BASE 0xC0000000

void* kmalloc(size_t size);
void kfree(void *ptr);
void *sbrk(size_t size);

//TODO move this out of here
uint32_t physical_to_virtual(uint32_t phys);