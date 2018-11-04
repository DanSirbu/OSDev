#pragma once

#include "types.h"
#include "multiboot.h"

#define PGSIZE 4096
#define PG_ROUND_DOWN(addr) (addr & ~(PGSIZE - 1))
#define PG_ROUND_UP(addr) PG_ROUND_DOWN((addr + (PGSIZE - 1)))

typedef uint32_t ptr_phy_t;

void frame_init(size_t mmap_addr, size_t mmap_len);
ptr_phy_t alloc_frame();
void frame_set_used(ptr_phy_t frame);
void free_frame_range(ptr_phy_t first_frame, uint32_t len);