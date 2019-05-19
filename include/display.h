#pragma once
#include "types.h"

void display_init(const uint32_t *framebuffer_addr, size_t width,
		  size_t height);

void display_update(const uint32_t *src);