#pragma once

#include "types.h"
#include "../include/multiboot.h"

typedef uint32_t ptr_phy_t;

void frame_init(size_t mmap_addr, size_t mmap_len);
ptr_phy_t alloc_frame();
void frame_set_used(ptr_phy_t frame);
void free_frame_range(ptr_phy_t first_frame, uint32_t len);