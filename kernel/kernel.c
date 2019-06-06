/*
 * Copyright (C) 2014  Arjun Sreedharan
 * License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html
 */
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
#include "time.h"
#include "spinlock.h"
#include "task.h"
#include "syscalls.h"
#include "display.h"

#include "test.h"
#include "ramfs.h"
#include "fs.h"
#include "vfs.h"
#include "vga.h"

//ramdisk
extern void initrd_init(size_t start, size_t size);

extern unsigned char keyboard_map[128];
extern void initialize_gdt();
extern void initialize_tss();
extern void kb_init();
extern void idt_init();
extern void kprint_newline();

uint8_t PIC1_INT = 0x00;
uint8_t PIC2_INT = 0x00;

void print_memory_map(size_t mmap_addr, size_t mmap_len)
{
	void *mmap = (void *)mmap_addr;
	void *mmap_end = mmap + mmap_len;

	debug_print("Memory Map\n");

	// clang-format off
	// unsigned long = sizeof(memory_map_t->size)
	for (; mmap < mmap_end; mmap += ((multiboot_memory_map_t *)mmap)->size + sizeof(unsigned long)) {
		// clang-format on

		multiboot_memory_map_t *mmap_cur =
			(multiboot_memory_map_t *)mmap;
		debug_print("Address: %8p-%8p, type %x\n",
			    (uint32_t)mmap_cur->addr,
			    (uint32_t)(mmap_cur->addr + mmap_cur->len - 1),
			    (uint32_t)mmap_cur->type);
	}
}

void err()
{
	__asm__("cli");
}
/*
void test_process1(size_t args)
{
	while (1) {
		cli();
		debug_print("a");
		sti();
		//schedule();
		//exec(NULL, (size_t)err);
	}
}

void test_process2(size_t args)
{
	while (1) {
		debug_print("b");
		schedule();
	}
}*/
//extern process_t task[];
//extern volatile process_t *current;

extern void get_func_info(uint32_t addr, char **name, char **file);
extern void dump_stack_trace();

void test2()
{
	dump_stack_trace();
}
void test1()
{
	test2();
}
extern char _kernel_end;
extern inode_t *(*dummy_find_child)();

size_t kern_max_address;
void kmain(multiboot_info_t *multiboot_info)
{
	cli();
	init_serial();
	debug_print("Flags: 0x%x\n", multiboot_info->flags);

	assert(multiboot_info->flags & MULTIBOOT_INFO_MODS);

	assert(multiboot_info->mods_count == 1);
	multiboot_module_t *modules = KERN_P2V(multiboot_info->mods_addr);
	size_t ramfs_location = (size_t)KERN_P2V(modules[0].mod_start);

	debug_print("Module start: 0x%x\n", modules[0].mod_start);
	debug_print("Module end: 0x%x\n", modules[0].mod_end);

	debug_print("Kernel ends at 0x%x\n", &_kernel_end);

	if (multiboot_info->flags & MULTIBOOT_INFO_ELF_SHDR) {
		//Section header table
		multiboot_elf_section_header_table_t sec_info =
			multiboot_info->u.elf_sec;
		assert(sec_info.num != 0);

		size_t sections_end =
			sec_info.addr + (sec_info.num * sec_info.size);

		debug_print("Section start: 0x%x\n", sec_info.addr);
		debug_print("Section end: 0x%x\n", sections_end);

		//Update kernel end pointer
		kern_max_address = MAX(kern_max_address, sections_end);
	}

	//Update kernel end pointer
	kern_max_address = MAX(kern_max_address, modules[0].mod_end);
	kern_max_address =
		MAX(kern_max_address, (size_t)&_kernel_end - KERN_BASE);
	kern_max_address = LPG_ROUND_UP(kern_max_address);

	initialize_gdt();
	initialize_tss();

	idt_init();

	screen_init();
	print_memory_map((size_t)KERN_P2V(multiboot_info->mmap_addr),
			 multiboot_info->mmap_length);

	//Switch to 2-level paging (2 level)
	debug_print("Paging init\n");
	paging_init(multiboot_info->mmap_addr, multiboot_info->mmap_length,
		    kern_max_address);
	debug_print("Paging init finished\n");
	//Malloc can now use the full heap
	kinit_malloc((size_t)KERN_HEAP_START, (size_t)KERN_HEAP_END);

	uint32_t *test = kmalloc(0x4000000);
	*test = 0x11223344;
	uint32_t *test2 = kmalloc(0x4000000);
	*test2 = 0x1234;
	kfree(test2);
	kfree(test);

	initramfs(ramfs_location);
	inode_t *fs_root_inode = ramfs_getRoot();
	mount("/", fs_root_inode);

	kb_init();

	inode_t *curInode = vfs_namei("/");
	inode_t *cur = NULL;
	uint32_t index = 0;
	do {
		cur = curInode->i_op->get_child(curInode, index);
		if (cur) {
			debug_print("Ino: %d\n", cur->ino);
		}
		index++;
	} while (cur != NULL);

	ramfs_dir_t dirs;
	file_t *file = vfs_open("/");
	int ret = vfs_read(file, &dirs, sizeof(dirs), 0);
	if (ret < 0) {
		debug_print("Read error\n");
	}
	for (uint32_t x = 0; x < dirs.numDir; x++) {
		debug_print("File: %s\n", dirs.dirents[x].name);
	}

	/*dirent_t file1;
	dirent_t file2;
	memcpy(&file1, ram_root->readdir(ram_root, 0), sizeof(dirent_t));
	memcpy(&file2, ram_root->readdir(ram_root, 1), sizeof(dirent_t));

	debug_print("File1: %s\n", file1.name);
	debug_print("File2: %s\n", file2.name);

	fs_node_t *hello = dirent_to_node(&file1);*/

	//mmap(0x90000000, 0x4000);
	//initrd_init(0x90000000, 0x4000);
	//char *abc = "AAAAAAABBBBBCCCCC";
	//device_write(0x1000, abc, 1);

	/* kmalloc testing code
	char *addr1 = kmalloc(10);
	addr1[0] = 0xAA;
	addr1[9] = 0xBB;
	char *addr2 = kmalloc(4080);

	kfree(addr1);
	size_t addr3 = kvmalloc(PGSIZE);
	kfree(addr3);
	size_t test1 = kmalloc(1);
	size_t test2 = kmalloc(1);
	size_t addr4 = kvmalloc(PGSIZE);
	*/
	timer_init(1000);

	int x = 0;
	spinlock_acquire(&x);

	debug_print("Lock acquired\n");
	spinlock_release(&x);
	debug_print("Lock released\n");

	//Processes
	//copy_process(test_process1);
	//copy_process(test_process2);

	//clone(test_process1, kmalloc(0x1000) + 0x1000);
	//clone(test_process2, kmalloc(0x1000) + 0x1000);

	syscalls_install();
	tasking_install();
	//sti();

	/*task_t *task1 = copy_task((size_t)test_process1, (size_t)NULL);
	task_t *task2 = copy_task((size_t)test_process2, (size_t)NULL);
	*/
	//make_task_ready(task1);
	//make_task_ready(task2);

	debug_print("Starting Tests\n");
	run_tests();
	debug_print("\nTests complete!\n");

//struct ModeInfoBlock *vbe_mode =
//	KERN_P2V(multiboot_info->vbe_mode);

//multiboot_info->framebuffer_height * multiboot_info->framebuffer_pitch;
#define FLAG_FRAMEBUFFER_EXISTS (1 << 12)
	assert(multiboot_info->flags & FLAG_FRAMEBUFFER_EXISTS);
	mmap_flags_t map_flags = { .IGNORE_FRAME_REUSE = 1,
				   .MAP_IMMEDIATELY = 1 };
	mmap_addr(KERN_IO_BASE, (uint32_t)multiboot_info->framebuffer_addr,
		  PG_ROUND_UP(multiboot_info->framebuffer_pitch *
			      multiboot_info->framebuffer_height),
		  map_flags);

	debug_print("Initializing display\n");
	display_init((const uint32_t *)KERN_IO_BASE,
		     multiboot_info->framebuffer_width,
		     multiboot_info->framebuffer_height);

	debug_print("Enabling Interrupts\n");
	sti();

	debug_print("Starting Init\n");

	const char *filename = "/init";
	char *args[] = { filename, NULL };
	char *envs[] = { "HOME", "/", NULL };
	execve(filename, args, envs);

	/*task_t *proc1 = &task[1];
	void *tmpStack = kmalloc(0x100) + 0x100;
	current->context =
		(context_t *)tmpStack; //Does not matter, we won't return here
	current->state = 100;
	schedule();*/
	//switch_context((context_t **)&current->context, proc1->context);

	/*while (1) {
		schedule();
	}*/

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