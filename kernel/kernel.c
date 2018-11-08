/*
 * Copyright (C) 2014  Arjun Sreedharan
 * License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html
 */
#include "acpi.h"
#include "e1000.h"
#include "keyboard_map.h"
#include "kmalloc.h"
#include "multiboot.h"
#include "pci.h"
#include "rtl8139.h"
#include "screen.h"
#include "serial.h"
#include "string.h"
#include "p_allocator.h"
#include "mmu.h"
#include "device.h"

//ramdisk
extern void initrd_init(size_t start, size_t size);

#define VIRT_TO_PHYS_ADDR(x) (x - 0xc0000000)
typedef unsigned int u32;

extern unsigned char keyboard_map[128];
extern void initialize_gdt();
extern void kb_init();
extern void paging_init();
extern void idt_init();
extern void kprint_newline();

uint8_t PIC1_INT = 0x01;
uint8_t PIC2_INT = 0x00;

void memory_map_handler(u32 mmap_addr, u32 mmap_len)
{
	// Must add KERN_BASE because mmap is a physical address
	void *mmap = (void *)mmap_addr + KERN_BASE;
	void *mmap_end = mmap + mmap_len;

	kpanic("Memory Map\n");

	// clang-format off
	// unsigned long = sizeof(memory_map_t->size)
	for (; mmap < mmap_end; mmap += ((memory_map_t *)mmap)->size + sizeof(unsigned long)) {
		// clang-format on

		memory_map_t *mmap_cur = (memory_map_t *)mmap;
		kpanic_fmt("Address: %8p-%8p, type %x\n",
			   mmap_cur->base_addr_low,
			   (mmap_cur->base_addr_low + mmap_cur->length_low - 1),
			   mmap_cur->type);
	}
}

void kmain(multiboot_info_t *multiboot_info)
{
	memory_map_handler(multiboot_info->mmap_addr,
			   multiboot_info->mmap_length);
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
	kfree(ptr);
	*/

	//Switch to more advanced paging (2 level)
	kpanic_fmt("Paging init\n");
	paging_init(multiboot_info->mmap_addr, multiboot_info->mmap_length);
	kpanic_fmt("Paging init finished\n"); // Malloc now works

	mmap(0x90000000, 0x4000);
	initrd_init(0x90000000, 0x4000);

	mmap(0xA0000000, 0x00F00000);

	char *abc = "AAAAAAABBBBBCCCCC";
	device_write(0x1000, abc, 1);
	//char *test = (char *)0xA0000000;
	//char asd = *test; //Page fault

	//assert(1 == 2); //Test assert

	//Testing
	/*frame_set_used(0x0);
	frame_set_used(0x00700000);
	frame_set_used(0x0efdffff);
	frame_set_used(0x0ffe0000);*/

	// acpi_init();
	//pci_find_devices(); // PCI init

	// pci_write_field(0x80861237, 0x08, 1, 0xFFFFFFFF); //Test out pci read and
	// write

	// ethernet_main();
	//RTL8139_Init();

	// clang-format off
	while (1);
	// clang-format on
}
