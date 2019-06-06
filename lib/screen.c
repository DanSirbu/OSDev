#include "screen.h"
#include "io.h"

static char *vidptr = KERN_P2V(0x000b8000);

/* current cursor location */
unsigned int current_loc = 0;

void disable_cursor()
{
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}
void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	uint32_t newLoc = current_loc + (line_size - current_loc % (line_size));

	//Clear out the rest of the line
	for (uint32_t x = current_loc; x < newLoc && x < SCREENSIZE / 2; x++) {
		kprint_char(' ');
	}

	current_loc = newLoc;
	if (current_loc > SCREENSIZE / 2) {
		current_loc = 0;
	}
}

void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = 0x07;
	}
	current_loc = 0;
}
void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = 0x07;
	}
}
void kprint_char(char a)
{
	vidptr[current_loc++] = a;
	vidptr[current_loc++] = 0x07;
}

void screen_init()
{
	disable_cursor();
	clear_screen();
}