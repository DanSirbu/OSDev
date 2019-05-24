#pragma once
#include "types.h"
#include "kmalloc.h"

#define KERN_BASE /*      */ 0xC0000000

//Leave some pages unmapped after the heap so we know if we go past it
#define KERN_HEAP_START /* */ 0xD0000000
#define KERN_HEAP_END /*  */ (0xE0000000 - 0x3000)

#define COPY_PAGE_SOURCE 0xE0000000
#define COPY_PAGE_DEST 0xE0001000

#define KERN_IO_BASE /*   */ 0xE0002000
#define KERN_IO_END /*    */ 0xE0F00000

#define USTACKTOP 0xB0000000
#define USTACKTOP2 0xAFFFE000

//Also defined in p_allocator.h
#define PGSIZE 4096
#define PGMASK 0xFFF
#define LPGMASK 0x3FFFFF

#define LPGSIZE 0x00400000 //Large page size = 4MB

#define PG_ROUND_DOWN(addr) (addr & ~(PGMASK))
#define PG_ROUND_UP(addr) PG_ROUND_DOWN((addr + (PGSIZE - 1)))

#define LPG_ROUND_DOWN(addr) (addr & ~(LPGMASK))
#define LPG_ROUND_UP(addr) (LPG_ROUND_DOWN((addr) + (LPGSIZE - 1)))

#define FRAME_TO_ADDR(frame) ((pptr_t)((frame) << POFFSHIFT))
#define ADDR_TO_FRAME(addr) ((addr) >> POFFSHIFT)

#define PTE_P 1

//Gets page directory index
#define PDX(addr) ((addr) >> 22)
//Gets page table index
#define PTX(addr) (((addr) >> 12) & 0x3FF)
#define POFF(addr) ((addr)&0xFFF)

#define PTSHIFT 22
#define POFFSHIFT 12

//Gets the address to put in the page entry
#define PTE_ADDR(addr) ((addr) >> POFFSHIFT)

//Ignore that the physical page is already in use
#define FLAG_IGNORE_PHY_REUSE 1

#define KERN_BASE_PAGE_NO (KERN_BASE >> 22)

typedef struct {
	unsigned int present : 1;
	unsigned int rw : 1;
	unsigned int user : 1;
	unsigned int writethrough : 1;
	unsigned int cachedisable : 1;
	unsigned int accessed : 1;
	unsigned int dirty : 1;
	unsigned int pat : 1;
	unsigned int global : 1;
	unsigned int unused : 3;
	unsigned int frame : 20;
} __attribute__((packed)) pte_t;

typedef union {
	struct {
		unsigned int present : 1;
		unsigned int rw : 1;
		unsigned int user : 1;
		unsigned int writethrough : 1;
		unsigned int cachedisable : 1;
		unsigned int accessed : 1;
		unsigned int zero : 1;
		unsigned int page_4k : 1;
		unsigned int global : 1;
		unsigned int unused : 3;
		unsigned int frame : 20;
	} __attribute__((packed));
	uint32_t bits;
} page_dir_entry_t;

typedef struct {
	pte_t pages[1024];
} page_table_t;

//Design inspired from Toaruos (https://github.com/klange/toaruos/blob/master/base/usr/include/kernel/task.h)
typedef struct {
	page_dir_entry_t actual_tables[1024]; //Actual entries the cpu uses
	page_table_t *tables[1024]; //pointer to virtual address of the tables
} __attribute__((packed)) page_directory_t;

static inline void invlpg(vptr_t addr)
{
	asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

typedef union {
	struct {
		uint8_t IGNORE_PAGE_MAPPED : 1;
		uint8_t IGNORE_FRAME_REUSE : 1;
		uint8_t MAP_IMMEDIATELY : 1;
	};
	uint8_t bits;
} mmap_flags_t;

extern page_directory_t *kernel_page_directory;

void mmap(size_t base, size_t len, mmap_flags_t flags);
void mmap_addr(vptr_t vaddr, pptr_t phyaddr, size_t len, mmap_flags_t flags);
void setPTE(page_directory_t *pgdir, vptr_t vaddr, pptr_t phyaddr);
void paging_init(size_t memory_map_base, size_t memory_map_full_len,
		 size_t kernel_end_phy_addr);
pptr_t virtual_to_physical(page_directory_t *pgdir, vptr_t addr);
void switch_page_directory(page_directory_t *new_pg_dir);
page_directory_t *clone_directory(page_directory_t *pgdir);
void free_user_mappings(page_directory_t *src_pg_dir);
void free_page_directory(page_directory_t *src_pg_dir);
void memcpy_frame_contents(pptr_t dst, pptr_t src);