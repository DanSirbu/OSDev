#include "display.h"

const uint32_t *framebuffer;
size_t display_width, display_height;

void display_init(const uint32_t *framebuffer_addr, size_t width, size_t height)
{
	framebuffer = framebuffer_addr;
	display_width = width;
	display_height = height;
}

//TODO, map framebuffer to userspace instead of copying it everytime
void display_update(const uint32_t *src)
{
	uint32_t *display_src = (uint32_t *)src;
	uint32_t *display_address = (uint32_t *)framebuffer;

	for (uint32_t y = 0; y < display_height; y++) {
		for (uint32_t x = 0; x < display_width; x++) {
			uint32_t *pixel = display_address + x;
			*pixel = *display_src;
			display_src++;
		}
		display_address += display_width;
	}
}