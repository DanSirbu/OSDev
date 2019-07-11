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
#include "syscall.h"
#include "display.h"
#include "elf.h"

#include "../tests/test.h"
#include "ramfs.h"
#include "fs.h"
#include "vfs.h"
#include "vga.h"
#include "dirent.h"
#include "debug.h"
#include "terminal.h"
#include "pipe.h"
#include "proc.h"

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

extern inode_t *keyboard_pipe;
extern inode_t *display_pipe;
extern inode_t *serial_pipe;

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
void testramfs()
{
	inode_t *curInode = vfs_namei("/");

	list_t *folderQueue = list_create();
	list_enqueue(folderQueue, vfs_open("/"));

	while (folderQueue->len != 0) {
		file_t *currentFolder = list_dequeue(folderQueue);

		ramfs_dir_t dir;
		vfs_read(currentFolder, &dir, 0, sizeof(dir));
		for (size_t x = 0; x < dir.num_dirs; x++) {
			char concatstr[255];
			if (currentFolder->path[strlen(currentFolder->path) -
						1] != '/') {
				strconcat(concatstr, currentFolder->path, "/");
			} else {
				strcpy(concatstr, currentFolder->path);
			}
			strconcat(concatstr, concatstr, dir.dirents[x].name);

			file_t *curFile = vfs_open(concatstr);

			debug_print("%s INO: %d\n", concatstr,
				    curFile->f_inode->ino);
			assert(curFile->f_inode->ino == dir.dirents[x].ino);

			if (curFile->f_inode->type == FS_DIRECTORY) {
				list_enqueue(folderQueue, curFile);
			} else {
				vfs_close(curFile);
			}
		}

		vfs_close(currentFolder);
	}
}
extern void get_func_info(uint32_t addr, char **name, char **file);

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
	void *ramfs_location = KERN_P2V(modules[0].mod_start);

	debug_print("Module start: 0x%x\n", modules[0].mod_start);
	debug_print("Module end: 0x%x\n", modules[0].mod_end);

	debug_print("Kernel ends at 0x%x\n", &_kernel_end);

	if (multiboot_info->flags & MULTIBOOT_INFO_ELF_SHDR) {
		size_t elf_end_address;
		parse_elf_sections(&multiboot_info->u.elf_sec,
				   &elf_end_address);
		//Update kernel end pointer
		kern_max_address = MAX(kern_max_address, elf_end_address);
	}
	//Update kernel end pointer
	kern_max_address = MAX(kern_max_address, modules[0].mod_end);
	kern_max_address =
		MAX(kern_max_address, (size_t)&_kernel_end - KERN_BASE);
	kern_max_address = LPG_ROUND_UP(kern_max_address);

	initialize_gdt();
	initialize_tss();

	idt_init();

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

	timer_init(1000);

	int x = 0;
	spinlock_acquire(&x);

	debug_print("Lock acquired\n");
	spinlock_release(&x);
	debug_print("Lock released\n");

	syscalls_install();
	tasking_install();

	print(LOG_INFO, "Initializing ramfs\n");
	initramfs(ramfs_location);
	inode_t *fs_root_inode = ramfs_getRoot();
	mount("/", fs_root_inode);

	//Adding more folders
	vfs_mkdir("/", "dev");
	testramfs();

	print(LOG_INFO, "Initializing keyboard\n");
	kb_init();
	vfs_mkdir("/dev", "keyboard");
	assert(mount("/dev/keyboard", keyboard_pipe) == 0);

	print(LOG_INFO, "Initializing screen\n");
	initialize_terminal();
	vfs_mkdir("/dev", "screen");
	assert(mount("/dev/screen", display_pipe) == 0);

	print(LOG_INFO, "Initializing serial file\n");
	vfs_mkdir("/dev", "serial");
	init_serial_pipe();
	assert(mount("/dev/serial", serial_pipe) == 0);

	print(LOG_INFO, "Initializing /dev/null\n");
	vfs_mkdir("/dev", "null");
	inode_t *null_pipe = make_null_pipe();
	assert(mount("/dev/null", null_pipe) == 0);

	print(LOG_INFO, "Initializing /proc\n");
	vfs_mkdir("/", "proc");
	inode_t *proc_pipe = make_proc_pipe();
	assert(mount("/proc", proc_pipe) == 0);

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

	debug_print("Starting Init\n");

	char *filename = strdup("/init");
	char **args = copy_arr((char *[]){ filename, NULL });
	char **envs = copy_arr((char *[]){ "HOME", "/", NULL });

	assert(execve(filename, args, envs) == 0);

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