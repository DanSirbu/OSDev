#pragma once

#include "types.h"
#include "multiboot.h"

#define PGSIZE 4096
#define PG_ROUND_DOWN(addr) (addr & ~(PGSIZE - 1))
#define PG_ROUND_UP(addr) PG_ROUND_DOWN((addr + (PGSIZE - 1)))

void frame_init(size_t mmap_addr, size_t mmap_len);
pptr_t alloc_frame();
void frame_set_used(pptr_t frame, uint8_t reuse_ok);
void free_frame_range(pptr_t first_frame, uint32_t len);