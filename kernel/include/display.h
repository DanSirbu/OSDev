#pragma once
#include "sys/types.h"

void display_init(const uint32_t *framebuffer_addr, size_t width,
		  size_t height);

void display_clear();

void display_update(const uint32_t *src);
void drawCharacter(uint8_t character, size_t xpos, size_t ypos);