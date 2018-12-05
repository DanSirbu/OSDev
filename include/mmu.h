#pragma once
#include "types.h"
#include "kmalloc.h"

#define KERN_BASE 0xC0000000
#define KERN_IO_BASE (KERN_BASE - 0x10000000)

#define PTE_P 1

//Gets page directory index
#define PDX(addr) ((addr) >> 22)

//Gets page table index
#define PTX(addr) (((addr) >> 12) & 0x3FF)

#define POFF(addr) ((addr)&0xFFF)

//Gets the address to put in the page entry
#define PTE_ADDR(addr) ((addr) & ~0xFFF)

//Ignore that the physical page is already in use
#define FLAG_IGNORE_PHY_REUSE 1

typedef struct {
	uint32_t tables[1024]; //Physical tables
} page_directory_t;

void mmap(size_t base, size_t len);
void mmap_addr(size_t vaddr, size_t phyaddr, size_t len, uint8_t flags);
void setPTE(size_t vaddr, size_t phyaddr);