#include "../include/io.h"
#include "../include/trap.h"
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define ENTER_KEY_CODE 0x1C

void kb_init(void);
void keyboard_handler_main(void);

void kb_init(void)
{
	//register_handler(TRAP_KEYBOARD, keyboard_handler_main);
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	// outb(0x21 , 0xFD);
}
void keyboard_handler_main(void)
{
	/*
      unsigned char status;
      char keycode;

      // write EOI
      outb(0x20, 0x20);

      status = inb(KEYBOARD_STATUS_PORT);
      // Lowest bit of status will be set if buffer is not empty
      if (status & 0x01) {
              keycode = inb(KEYBOARD_DATA_PORT);
              if(keycode < 0)
                      return;

              if(keycode == ENTER_KEY_CODE) {
                      kprint_newline();
                      return;
              }

              //vidptr[current_loc++] = keyboard_map[(unsigned char) keycode];
              //vidptr[current_loc++] = 0x07;
      }
  */
}