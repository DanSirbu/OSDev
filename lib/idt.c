#include "../include/types.h"
#include "../include/boot.h"

extern void load_idt(unsigned long *idt_ptr);
extern void idt_0(void);

#define IDT_SIZE 256
#define KERNEL_CODE_SEGMENT_OFFSET 0x08
#define INTERRUPT_GATE 0x8e

struct idt_description_structure_t {
  u16 size; // in bytes
  u32 offset;
} __attribute__((packed)) idt_description_structure;
struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

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
	/*idt_address = (u32) keyboard_handler;
	IDT[0x21].offset_lowerbits = idt_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (idt_address & 0xffff0000) >> 16;
    */
   
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

	/* mask interrupts *///TODO remove this
	write_port(PIC1_DATA, 0xFF);
	write_port(PIC2_DATA, 0xFF);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT;
	idt_description_structure.size = sizeof(IDT) - 1;
	idt_description_structure.offset = (u32) IDT;

	load_idt((u64*) &idt_description_structure);
}

void sendEOI(uint32_t interrupt_no) {
    	if(interrupt_no >= 0x20 && interrupt_no <= 0x30) {
		int irqNum = interrupt_no - 0x20;
		if(irqNum > 0x7) {
			write_port(PIC2_CMD, END_OF_INTERRUPT);
		}
		write_port(PIC1_CMD, END_OF_INTERRUPT);
	}
}