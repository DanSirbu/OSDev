#pragma once
#include "../include/types.h"
#include "../include/kmalloc.h"

#define PTE_P 1

//Gets page directory index
#define PDX(addr) ((addr) >> 22)

//Gets page table index
#define PTX(addr) (((addr) >> 12) & 0x3FF)

#define POFF(addr) ((addr)&0xFFF)

//Gets the address to put in the page entry
#define PTE_ADDR(addr) ((addr) & ~0xFFF)

void mmap(size_t base, size_t len);
void mmap_addr(size_t vaddr, size_t phyaddr, size_t len);
void setPTE(size_t vaddr, size_t phyaddr);