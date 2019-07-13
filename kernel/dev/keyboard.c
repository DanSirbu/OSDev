#include "io.h"
#include "trap.h"
#include "circularqueue.h"
#include "fs.h"
#include "pipe.h"
#include "vfs.h"
#include "keyboard_map.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_COMMAND_REGISTER 0x64

void kb_init(void);
void keyboard_handler_main(int_regs_t *regs);

inode_t *keyboard_pipe;

void kb_init(void)
{
	keyboard_pipe = make_pipe(100);
	register_isr_handler(TRAP_KEYBOARD,
			     (isr_handler_t)keyboard_handler_main);
	uint8_t pic_mask = inb(PIC1_DATA);
	outb(PIC1_DATA, (pic_mask & ~(1 << 1)));
}

int prevSequence = 0;
void keyboard_handler_main(__attribute__((unused)) int_regs_t *regs)
{
	unsigned char status;
	uint8_t scancode;

	status = inb(KEYBOARD_STATUS_PORT);
	assert((status & 0x01));

	scancode = inb(KEYBOARD_DATA_PORT);

	if (scancode == 0xE0) {
		prevSequence = 1;
		return;
	}

	if (scancode != '\0') {
		file_t file;
		file.f_inode = keyboard_pipe;
		strncpy(file.path, KEYBOARD_DEVICE, sizeof(file.path));

		vfs_write(&file, &scancode, 0, 1); //Write scancode
	}
}