#include "sys/types.h"
#include "io.h"
#include "trap.h"

extern void load_idt(size_t *idt_ptr);
extern void idt_0(void);
extern uint8_t PIC1_INT;
extern uint8_t PIC2_INT;

#define IDT_SIZE 256
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define TASK_GATE_32 0x5
#define INTERRUPT_GATE_16 0x6
#define TRAP_GATE_16 0x7
#define INTERRUPT_GATE_32 0xE
#define TRAP_GATE_32 0xF

#define ICW4_8086 0x01 /* 8086/88 (MCS-80/85) mode */
#define END_OF_INTERRUPT 0x20

struct idt_description_structure_t {
	uint16_t size; // in bytes
	uint32_t offset;
} __attribute__((packed)) idt_description_structure;

struct IDT_entry {
	uint16_t offset_lowerbits;
	uint16_t selector;
	unsigned char zero;
	struct {
		uint8_t gatetype : 4;
		uint8_t storage_segment : 1;
		uint8_t dpl : 2;
		uint8_t present : 1;
	} type;
	uint16_t offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

void idt_init(void)
{
	uint32_t idt_address;
	uint32_t idt_0_address;

	/* populate IDT entry of keyboard's interrupt */
	idt_0_address = (uint32_t)idt_0;

	for (int x = 0; x < 256; x++) {
		idt_address =
			idt_0_address + (x * 0x10); // each handler is 16 bytes
		// aligned
		IDT[x].offset_lowerbits = idt_address & 0xFFFF;
		IDT[x].offset_higherbits = idt_address >> 16;

		IDT[x].selector = KERNEL_CODE_SEGMENT_OFFSET;
		IDT[x].zero = 0;

		IDT[x].type.gatetype = INTERRUPT_GATE_32;
		IDT[x].type.storage_segment = 0;
		IDT[x].type.dpl = 0;
		IDT[x].type.present = 1;
	}
	IDT[0x80].type.dpl = 3;
	/*     Ports
   *	 PIC1	PIC2
   *Command 0x20	0xA0
   *Data	 0x21	0xA1
   */

	/* ICW1 - begin initialization */
	outb(PIC1_CMD, 0x11);
	outb(PIC2_CMD, 0x11);

	/* ICW2 - remap offset address of IDT */
	outb(PIC1_DATA, 0x20);
	outb(PIC2_DATA, 0x28);

	/* ICW3 - setup cascading */
	outb(PIC1_DATA,
	     0x04); // Tell pic1 that pic2 is at pin 3 (0x0000 0100)
	outb(PIC2_DATA, 0x02); // Tell pic2 its cascade number is 2

	/* ICW4 - environment info */
	outb(PIC1_DATA, ICW4_8086);
	outb(PIC2_DATA, ICW4_8086);
	/* Initialization finished */

	/* mask interrupts */ // TODO remove this
	outb(PIC1_DATA, PIC1_INT);
	outb(PIC2_DATA, PIC2_INT);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT;
	idt_description_structure.size = sizeof(IDT) - 1;
	idt_description_structure.offset = (uint32_t)IDT;

	load_idt((size_t *)&idt_description_structure);
}

void sendEOI(uint32_t interrupt_no)
{
	if (interrupt_no >= 0x20 && interrupt_no <= 0x30) {
		int irqNum = interrupt_no - 0x20;
		if (irqNum > 0x7) {
			outb(PIC2_CMD, END_OF_INTERRUPT);
		}
		outb(PIC1_CMD, END_OF_INTERRUPT);
	}
}