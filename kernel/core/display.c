#include "display.h"
#include "font8x8_basic.h"
#include "serial.h"

const uint32_t *framebuffer;
size_t display_width, display_height;

void display_init(const uint32_t *framebuffer_addr, size_t width, size_t height)
{
	framebuffer = framebuffer_addr;
	display_width = width;
	display_height = height;
	debug_print("Display width %dx%d\n", width, height);
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

//TODO, optimize this
void drawCharacter(uint8_t character, size_t xpos, size_t ypos)
{
	uint32_t *topLeft =
		(uint32_t *)framebuffer + xpos + ypos * display_width;

	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			uint8_t pixelEnabled =
				font8x8_basic[character][y] & (1 << x);
			if (pixelEnabled) {
				topLeft[x] = 0xFFFFFFFF;
			} else {
				topLeft[x] = 0x0;
			}
		}

		topLeft += display_width;
	}
}
void setPixel(size_t x, size_t y, uint32_t value)
{
	uint32_t *pixel = (uint32_t *)framebuffer + y * display_width + x;
	*pixel = value;
}