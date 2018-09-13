/*
* Copyright (C) 2014  Arjun Sreedharan
* License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html
*/
#include "include/keyboard_map.h"
#include "include/serial.h"
#include "include/string.h"

#define VIRT_TO_PHYS_ADDR(x) (x - 0xc0000000)
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
extern void initialize_gdt();
extern void kmalloc();

/* current cursor location */
unsigned int current_loc = 0;
/* video memory begins at address 0xb8000 */
char *vidptr = (char*)0xc00b8000;

struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

struct idt_description_structure_t {
  u16 size; // in bytes
  u32 offset;
} __attribute__((packed)) idt_description_structure;

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21

#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define END_OF_INTERRUPT 0x20
void idt_init(void)
{
	u32 keyboard_address;
	u32 idt_address;
	u32 idt_0_address;
	
	/* populate IDT entry of keyboard's interrupt */
	idt_0_address = (u32) idt_0;

	for(int x = 0; x < 256; x++) {
		idt_address = idt_0_address + (x * 0x10);//each handler is 16 bytes aligned
		IDT[x].offset_lowerbits = idt_address & 0xffff;
		IDT[x].selector = KERNEL_CODE_SEGMENT_OFFSET;
		IDT[x].zero = 0;
		IDT[x].type_attr = INTERRUPT_GATE;
		IDT[x].offset_higherbits = (idt_address & 0xffff0000) >> 16;
	}
	idt_address = (u32) keyboard_handler;
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
	write_port(PIC1_CMD, 0x11);
	write_port(PIC2_CMD, 0x11);

	/* ICW2 - remap offset address of IDT */
	write_port(PIC1_DATA, 0x20);
	write_port(PIC2_DATA, 0x28);

	/* ICW3 - setup cascading */
	write_port(PIC1_DATA, 0x04); //Tell pic1 that pic2 is at pin 3 (0x0000 0100)
	write_port(PIC2_DATA, 0x02); //Tell pic2 its cascade number is 2

	/* ICW4 - environment info */
	write_port(PIC1_DATA, ICW4_8086);
	write_port(PIC2_DATA, ICW4_8086);
	/* Initialization finished */

	/* mask interrupts *///TODO
	write_port(PIC1_DATA, 0x0);
	write_port(PIC2_DATA, 0x0);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT;
	idt_description_structure.size = sizeof(IDT) - 1;
	idt_description_structure.offset = (u32) IDT;

	load_idt((u64*) &idt_description_structure);
}

void kb_init(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	//write_port(0x21 , 0xFD);
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
#define CMOS_PORT 0x70
#define CMOS_PORT_INOUT 0x71

char read_cmos(u8 cmos_reg) {
	write_port(CMOS_PORT, cmos_reg); //Must always reselect before reading because it seems reading clears the selection
	return read_port(CMOS_PORT_INOUT);
}

void print_time() {
	/* From https://wiki.osdev.org/CMOS#Accessing_CMOS_Registers
	Register  Contents
	0x00      Seconds
 	0x02      Minutes
	0x04      Hours
	0x06      Weekday
	0x07      Day of Month
	0x08      Month
	0x09      Year
	0x32      Century (maybe)
	0x0A      Status Register A
	0x0B      Status Register B
	*/
	u64 value = (u64) read_cmos(0);
	kpanic_fmt("%d\n", value);
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

	initialize_gdt();
	idt_init();
	kb_init();

	init_serial();
	kpanic_fmt("Serial initialized\n");

	int a = 5 / 0;
	kmalloc();
	while(1);
}
#define IRQ_PIT 0x20

void interrupt_handler(u32 cr2, u32 edi, u32 esi, u32 ebp, u32 esp, u32 ebx, u32 edx, u32 ecx, u32 eax, u32 interrupt_no, u32 error_code, u32 eip) {
	kpanic_fmt("Interrupt %d at 0x%x, error %d\n", (u64) interrupt_no, (u64) eip, (u64) error_code);
	if(interrupt_no == 0) { //Don't know what to do yet so just ignore
		eip += 1;
	} else if(interrupt_no == IRQ_PIT) {
		print_time();
	}

	if(interrupt_no >= 0x20 && interrupt_no <= 0x30) {
		int irqNum = interrupt_no - 0x20;
		if(irqNum > 0x7) {
			write_port(PIC2_CMD, END_OF_INTERRUPT);
		}
		write_port(PIC1_CMD, END_OF_INTERRUPT);
	}
}
struct PD {
	unsigned char present: 1;
	unsigned char writable: 1;
	unsigned char user: 1;
	unsigned char write_through : 1;
	unsigned char cache_disabled : 1;
	unsigned char accessed : 1;
    unsigned char zero : 1;
	unsigned char size_4mb : 1;
	unsigned char g: 1;
	unsigned char avail: 3;
	unsigned int address: 20;
};