#include "io.h"
#include "trap.h"
#include "circularqueue.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define ENTER_KEY_CODE 0x1C

void kb_init(void);
void keyboard_handler_main(int_regs_t *regs);

CircularQueue *keyboard_buffer;

void kb_init(void)
{
	keyboard_buffer = CircularQueueCreate(100);
	register_isr_handler(TRAP_KEYBOARD,
			     (isr_handler_t)keyboard_handler_main);
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	uint8_t pic_mask = inb(PIC1_DATA);
	outb(PIC1_DATA, (pic_mask & ~(1 << 1)));
}
void keyboard_handler_main(__attribute__((unused)) int_regs_t *regs)
{
	unsigned char status;
	char keycode;

	// write EOI
	outb(0x20, 0x20);

	status = inb(KEYBOARD_STATUS_PORT);
	// Lowest bit of status will be set if buffer is not empty
	if (status & 0x01) {
		keycode = inb(KEYBOARD_DATA_PORT);
		if (keycode < 0)
			return;

		bool success = CircularQueueEnQueue(keyboard_buffer, keycode);
		assert(success);
		//debug_print("Keyboard: %d\n", keycode);
	}
}