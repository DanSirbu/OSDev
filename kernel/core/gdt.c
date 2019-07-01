// Taken from https://github.com/cstack/osdev
extern void *kernel_stack_lowest_address;
extern void tss_flush();
extern void asm_lgdt();
extern void update_segment_registers();

#define KERNEL_STACK_SIZE 4096

#include "sys/types.h"

// A struct describing a Task State Segment.
struct tss_entry_struct {
	uint32_t prev_tss; // The previous TSS - if we used hardware task switching
		// this would form a linked list.
	uint32_t esp0; // The stack pointer to load when we change to kernel mode.
	uint32_t ss0; // The stack segment to load when we change to kernel mode.
	uint32_t esp1; // everything below here is unusued now..
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap_base;
} __attribute__((packed)) tss;

// As described here: http://wiki.osdev.org/Global_Descriptor_Table#Structure
struct segment_descriptor_t {
	uint16_t limit_0_15; // bits 0-15 of limit
	uint16_t base_0_15;
	uint8_t base_16_23;
	uint8_t access_byte;
	uint8_t flags_and_limit_16_19;
	uint8_t base_24_31;
} __attribute__((packed));

#define NULL_SEGMENT_INDEX 0
#define KERNEL_CODE_SEGMENT_INDEX 1
#define KERNEL_DATA_SEGMENT_INDEX 2
#define USER_CODE_SEGMENT_INDEX 3
#define USER_DATA_SEGMENT_INDEX 4
#define TASK_STATE_SEGMENT_INDEX 5

struct segment_descriptor_t gdt[6];

// a pointer to the global descriptor table
// passed by reference to the LGDT instruction
struct gdt_description_structure_t {
	uint16_t size; // in bytes
	uint32_t offset;
} __attribute__((packed)) gdt_pointer;

#define GDT_PRESENT (1 << 15)
#define GDT_RING(ring_level) ((ring_level) << (5 + 8))
#define GDT_CODE_DATA_SEGMENT (1 << (4 + 8))
#define GDT_EXECUTABLE (1 << (3 + 8))

#define GDT_DIRECTION_UP 0
#define GDT_DIRECTION_DOWN 1
#define GDT_DIRECTION(direction) ((direction) << (2 + 8))

#define GDT_RW (1 << (1 + 8))

#define GDT_PAGE_GRANULARITY (1 << 7)
#define GDT_PROTECTED_MODE_SELECTOR (1 << 6)

void initialize_segment_descriptor(int index, uint32_t base, uint32_t limit,
				   uint16_t flags)
{
	gdt[index].limit_0_15 = limit & 0xFFFF;
	gdt[index].base_0_15 = base & 0xFFFF;
	gdt[index].base_16_23 = (base >> 16) & 0xFF;
	gdt[index].access_byte = flags >> 8;
	gdt[index].flags_and_limit_16_19 = (flags & 0xF0) | (limit & 0xF);
	gdt[index].base_24_31 = (base >> 24) & 0xFF;
}
void initialize_tss()
{
	uint32_t tss_base = (uint32_t)&tss;
	uint32_t tss_limit = sizeof(tss);

	uint16_t tss_flags =
		GDT_PRESENT | GDT_RING(3) | GDT_EXECUTABLE | 0x1 << 8;
	initialize_segment_descriptor(TASK_STATE_SEGMENT_INDEX, tss_base,
				      tss_limit, tss_flags);

	uint16_t kernel_data_segment_offset =
		KERNEL_DATA_SEGMENT_INDEX * 8; //Each entry is 8 bytes
	tss.ss0 = kernel_data_segment_offset;
	tss.esp0 = 0; //Initialized by set_kernel_stack

	tss.cs = 0x0b;
	tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;
	tss_flush();
}

void initialize_gdt()
{
	gdt_pointer.size = sizeof(gdt) - 1;
	gdt_pointer.offset = (uint32_t)gdt;

	initialize_segment_descriptor(NULL_SEGMENT_INDEX, 0, 0, 0);

	uint16_t kernel_code_flags = GDT_PRESENT | GDT_RING(0) |
				     GDT_CODE_DATA_SEGMENT | GDT_EXECUTABLE |
				     GDT_RW | GDT_PAGE_GRANULARITY |
				     GDT_PROTECTED_MODE_SELECTOR;

	initialize_segment_descriptor(KERNEL_CODE_SEGMENT_INDEX, 0, 0xFFFFFFFF,
				      kernel_code_flags);

	initialize_segment_descriptor(KERNEL_DATA_SEGMENT_INDEX, 0, 0xFFFFFFFF,
				      kernel_code_flags & (~GDT_EXECUTABLE));

	uint16_t user_code_flags = GDT_PRESENT | GDT_RING(3) |
				   GDT_CODE_DATA_SEGMENT | GDT_EXECUTABLE |
				   GDT_RW | GDT_PAGE_GRANULARITY |
				   GDT_PROTECTED_MODE_SELECTOR;

	initialize_segment_descriptor(USER_CODE_SEGMENT_INDEX, 0, 0xFFFFFFFF,
				      user_code_flags);
	initialize_segment_descriptor(USER_DATA_SEGMENT_INDEX, 0, 0xFFFFFFFF,
				      user_code_flags & (~GDT_EXECUTABLE));

	//gdt[KERNEL_CODE_SEGMENT_INDEX].access_byte = 0b10011010;

	//gdt[KERNEL_DATA_SEGMENT_INDEX].access_byte = 0b10010010;

	//gdt[USER_CODE_SEGMENT_INDEX].access_byte = 0b11111010;

	//gdt[USER_DATA_SEGMENT_INDEX].access_byte = 0b11110010;

	asm_lgdt(&gdt_pointer);

	update_segment_registers();
}
void set_kernel_stack(size_t stack)
{
	tss.esp0 = (uint32_t)stack;
}