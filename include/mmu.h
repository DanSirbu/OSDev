#pragma once
#include "types.h"
#include "kmalloc.h"

#define KERN_BASE /*      */ 0xC0000000

#define KERN_HEAP_START /**/ 0xD0000000
//Leave some pages unmapped after the heap so we know if we go past it
#define KERN_HEAP_END /*  */ (0xE0000000 - 0x3000)

#define KERN_IO_BASE /*   */ 0xE0000000

//Also defined in p_allocator.h
#define PGSIZE 4096
#define PG_ROUND_DOWN(addr) (addr & ~(PGSIZE - 1))
#define PG_ROUND_UP(addr) PG_ROUND_DOWN((addr + (PGSIZE - 1)))

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
void paging_init(size_t memory_map_base, size_t memory_map_full_len);