/*
* Copyright (C) 2014  Arjun Sreedharan
* License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html
*/
#include "include/keyboard_map.h"
#include "include/serial.h"
#include "include/string.h"
#include "include/kmalloc.h"
#include "include/e1000.h"
#include "include/screen.h"
#include "include/multiboot.h"

#define VIRT_TO_PHYS_ADDR(x) (x - 0xc0000000)
typedef unsigned int u32;

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern void initialize_gdt();
extern void kb_init();
extern void paging_init();
extern void idt_init();
extern void kprint_newline();
extern void sendEOI(uint32_t interrupt_no);

uint8_t PIC1_INT = 0x01;
uint8_t PIC2_INT = 0x00;

void memory_map_handler(u32 mmap_addr, u32 mmap_len) {
	//Must add KERN_BASE because mmap is a physical address
	void *mmap = (void*) mmap_addr + KERN_BASE; //-4 Because size starts at -4
	void *mmap_end = mmap + mmap_len;

	kpanic("Memory Map\n");
	for(; mmap < mmap_end; mmap += ((memory_map_t*) mmap)->size + sizeof(unsigned long)) { //unsigned long = sizeof(memory_map_t->size)
	memory_map_t *mmap_cur = (memory_map_t*) mmap;

		kpanic_fmt("Address: %p-%p, type %x\n", mmap_cur->base_addr_low, (mmap_cur->base_addr_low + mmap_cur->length_low - 1), mmap_cur->type);
	}
}
void kmain(multiboot_info_t *multiboot_info)
{
	memory_map_handler(multiboot_info->mmap_addr, multiboot_info->mmap_length);
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
	/*
	int a = 5 / 0;
	kmalloc(10);
	void* ptr = kmalloc(0x100);
	kfree(ptr);*/
	//Initialize paging
	kpanic_fmt("Paging init\n");
	paging_init();
	kpanic_fmt("Paging init finished\n");
	//Malloc and ethernet now work
	
	ethernet_main();
	while(1);
}

void interrupt_handler(u32 cr2, u32 edi, u32 esi, u32 ebp, u32 esp, u32 ebx, u32 edx, u32 ecx, u32 eax, const u32 interrupt_no, u32 error_code, u32 eip) {
	if(interrupt_no < 32) {
		kpanic_fmt("Exception %d (%s) at 0x%x, error %d\n", (u64) interrupt_no, exceptions_string[interrupt_no], (u64) eip, (u64) error_code);
	} else {
		kpanic_fmt("Interrupt %d (%s) at 0x%x, error %d\n", (u64) interrupt_no - 32, interrupts_string[interrupt_no - 32], (u64) eip, (u64) error_code);
	}

	if(interrupt_no == 0) { //Don't know what to do yet so just ignore
		eip += 1;
	}
	
	/* else if(interrupt_no == IRQ_PIT) {
		print_time();
	}*/
	if(interrupt_no == 11 + 32) {
		E1000_Interrupt();
	}

	sendEOI(interrupt_no);
}