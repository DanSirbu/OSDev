/*
* Copyright (C) 2014  Arjun Sreedharan
* License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html
*/
#include "keyboard_map.h"
#include "serial.c"

typedef unsigned int u32;

/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define ENTER_KEY_CODE 0x1C

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);
extern void idt_0(void);

/* current cursor location */
unsigned int current_loc = 0;
/* video memory begins at address 0xb8000 */
char *vidptr = (char*)0xb8000;

struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];


void idt_init(void)
{
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_0_address;
	unsigned long idt_ptr[2];
	
	/* populate IDT entry of keyboard's interrupt */
	idt_0_address = (unsigned long) idt_0;

	for(int x = 0; x < 256; x++) {
		idt_address = idt_0_address + x * 6;//each handler is 6 bytes so we can calculate all handlers address
		IDT[x].offset_lowerbits = idt_address + x*6 & 0xffff;
		IDT[x].selector = KERNEL_CODE_SEGMENT_OFFSET;
		IDT[x].zero = 0;
		IDT[x].type_attr = INTERRUPT_GATE;
		IDT[x].offset_higherbits = (idt_address & 0xffff0000) >> 16;
	}
	idt_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = idt_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (idt_address & 0xffff0000) >> 16;

	/*     Ports
	*	 PIC1	PIC2
	*Command 0x20	0xA0
	*Data	 0x21	0xA1
	*/

	/* ICW1 - begin initialization */
	write_port(0x20 , 0x11);
	write_port(0xA0 , 0x11);

	/* ICW2 - remap offset address of IDT */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	write_port(0x21 , 0x20);
	write_port(0xA1 , 0x28);

	/* ICW3 - setup cascading */
	write_port(0x21 , 0x00);
	write_port(0xA1 , 0x00);

	/* ICW4 - environment info */
	write_port(0x21 , 0x01);
	write_port(0xA1 , 0x01);
	/* Initialization finished */

	/* mask interrupts */
	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}

void kb_init(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(0x21 , 0xFD);
}

void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = 0x07;
	}
}

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
}

void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = 0x07;
	}
}

void keyboard_handler_main(void)
{
	unsigned char status;
	char keycode;

	/* write EOI */
	write_port(0x20, 0x20);

	status = read_port(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		keycode = read_port(KEYBOARD_DATA_PORT);
		if(keycode < 0)
			return;

		if(keycode == ENTER_KEY_CODE) {
			kprint_newline();
			return;
		}

		vidptr[current_loc++] = keyboard_map[(unsigned char) keycode];
		vidptr[current_loc++] = 0x07;
	}
}

void kmain(void)
{
	const char *str = "my first kernel with keyboard support";
	clear_screen();
	kprint(str);
	kprint_newline();
	kprint_newline();

	idt_init();
	kb_init();

	init_serial();
	int a = 5 / 0;
	while(1);
}
void itoa(u32 number, char *str, u32 base);
void interrupt_handler(u32 cr2, u32 edi, u32 esi, u32 ebp, u32 esp, u32 ebx, u32 edx, u32 ecx, u32 eax, u32 interrupt_no, u32 error_code, u32 eip) {
	char buf[20];

	//itoa(interrupt_no, buf, 16);
	kpanic_fmt("interrupt 0x%x\n", 16);
	kpanic_fmt("interrupt %d\n", 16);
	if(interrupt_no == 0) {
		eip += 1;//Ignore divide by zero error
	}
}
u32 strlen(char *str) {
	int i = 0;
	while(str[i] != '\0') { i++; }
	return i;
}
void reverse(char *str) {
	char *start = str;
	char *end = str + strlen(str) - 1;
	char temp;

	while(end > start) {
		temp = *start;
		*start = *end;
		*end = temp;

		start++;
		end--;
	}
}
void itoa(u32 number, char *str, u32 base) {
	int curNumber = number;
	if(number == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}
	int i = 0;
	while(curNumber != 0) {
		int rem = curNumber % base;
		if(rem < 10) {
		str[i++] = rem + 0x30;
		} else {
			str[i++] = rem + 0x51;
		}
		curNumber /= base;
	}
	str[++i] = '\0';

	reverse(str);
}