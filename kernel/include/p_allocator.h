#pragma once

#include "sys/types.h"
#include "multiboot.h"

void frame_init(size_t mmap_addr, size_t mmap_len);
size_t alloc_frame();
void free_frame(size_t frame_addr);
void frame_set_used(size_t frame, uint8_t reuse_ok);
void free_frame_range(size_t first_frame, uint32_t len);