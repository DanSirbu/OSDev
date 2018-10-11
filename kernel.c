/*
* Copyright (C) 2014  Arjun Sreedharan
* License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html
*/
#include "include/keyboard_map.h"
#include "include/serial.h"
#include "include/string.h"
#include "include/kmalloc.h"
#include "include/e1000.h"

#define VIRT_TO_PHYS_ADDR(x) (x - 0xc0000000)
typedef unsigned int u32;

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern void initialize_gdt();
extern void kb_init();
extern void paging_init();
extern void idt_init();
extern void kprint(char * str);
extern void kprint_newline();
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void sendEOI(uint32_t interrupt_no);

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
	/*
	int a = 5 / 0;
	kmalloc(10);
	void* ptr = kmalloc(0x100);
	kfree(ptr);*/

	//Initialize paging
	kpanic_fmt("Paging init\n");
	paging_init();
	kpanic_fmt("Paging init finished\n");
	
	ethernet_main();
	//Page fault at writeCommand+34, 0xc01019da, write at 0xfebc0050
	while(1);
}

void interrupt_handler(u32 cr2, u32 edi, u32 esi, u32 ebp, u32 esp, u32 ebx, u32 edx, u32 ecx, u32 eax, u32 interrupt_no, u32 error_code, u32 eip) {
	kpanic_fmt("Interrupt %d at 0x%x, error %d\n", (u64) interrupt_no, (u64) eip, (u64) error_code);
	
	if(interrupt_no == 0) { //Don't know what to do yet so just ignore
		eip += 1;
	}
	/* else if(interrupt_no == IRQ_PIT) {
		print_time();
	}*/

	sendEOI(interrupt_no);
}