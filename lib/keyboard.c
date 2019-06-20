#include "io.h"
#include "trap.h"
#include "circularqueue.h"
#include "fs.h"
#include "pipe.h"
#include "vfs.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define ENTER_KEY_CODE 0x1C

void kb_init(void);
void keyboard_handler_main(int_regs_t *regs);

inode_t *keyboard_pipe;

char kbd_us[128] = {
	//From toaruos
	0,    27,   '1', '2',  '3', '4',  '5',  '6', '7',
	'8',  '9',  '0', '-',  '=', '\b', '\t', /* tab */
	'q',  'w',  'e', 'r',  't', 'y',  'u',  'i', 'o',
	'p',  '[',  ']', '\n', 0, /* control */
	'a',  's',  'd', 'f',  'g', 'h',  'j',  'k', 'l',
	';',  '\'', '`', 0, /* left shift */
	'\\', 'z',  'x', 'c',  'v', 'b',  'n',  'm', ',',
	'.',  '/',  0, /* right shift */
	'*',  0, /* alt */
	' ', /* space */
	0, /* caps lock */
	0, /* F1 [59] */
	0,    0,    0,   0,    0,   0,    0,    0,   0, /* ... F10 */
	0, /* 69 num lock */
	0, /* scroll lock */
	0, /* home */
	0, /* up */
	0, /* page up */
	'-',  0, /* left arrow */
	0,    0, /* right arrow */
	'+',  0, /* 79 end */
	0, /* down */
	0, /* page down */
	0, /* insert */
	0, /* delete */
	0,    0,    0,   0, /* F11 */
	0, /* F12 */
	0, /* everything else */
};
#define KEY_UP_MASK 0x80

void kb_init(void)
{
	keyboard_pipe = make_pipe(100);
	register_isr_handler(TRAP_KEYBOARD,
			     (isr_handler_t)keyboard_handler_main);
	uint8_t pic_mask = inb(PIC1_DATA);
	outb(PIC1_DATA, (pic_mask & ~(1 << 1)));
}
void keyboard_handler_main(__attribute__((unused)) int_regs_t *regs)
{
	unsigned char status;
	char keycode;

	status = inb(KEYBOARD_STATUS_PORT);
	while (status & 0x01) {
		keycode = inb(KEYBOARD_DATA_PORT);
		if (keycode < 0)
			return;

		keycode &= ~KEY_UP_MASK; //Remove uppercase
		vfs_write(keyboard_pipe, &keycode, 0, 1);

		status = inb(KEYBOARD_STATUS_PORT);
	}
}