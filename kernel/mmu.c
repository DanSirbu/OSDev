#include "types.h"
#include "p_allocator.h"
#include "mmu.h"
#include "serial.h"
#include "trap.h"
#include "asm.h"

extern char _kernel_end;

void page_fault_handler(int_regs_t *regs);

extern page_directory_t *boot_page_directory;
page_directory_t *current_directory;
page_directory_t *kernel_page_directory;

int PAGING_INIT = 0;
/*
 * Returns the physical address
 * 
 * If paging has not been initialized (PAGING_INIT = 0), then this method returns ONLY the address of frames on the heap
 */
pptr_t virtual_to_physical(page_directory_t *pgdir, vptr_t addr)
{
	if (!PAGING_INIT) {
		//Initial heap is right after the kernel_end
		return (addr - KERN_HEAP_START) +
		       ((size_t)&_kernel_end - KERN_BASE);
	}
	size_t pdx = PDX(addr);
	size_t ptx = PTX(addr);

	return (pgdir->tables[pdx]->pages[ptx].frame << POFFSHIFT) |
	       (addr & 0xFFF);
}
void switch_page_directory(page_directory_t *new_pg_dir)
{
	pptr_t pgdir_phy =
		virtual_to_physical(kernel_page_directory, (vptr_t)new_pg_dir);
	current_directory = new_pg_dir;
	LoadNewPageDirectory(pgdir_phy);
}
int is_mapped(page_directory_t *pg, vptr_t addr)
{
	size_t pdx = PDX(addr);
	size_t ptx = PTX(addr);

	if (pg->actual_tables[pdx].present) {
		return pg->tables[pdx]->pages[ptx].present;
	}

	return false;
}

//Just map it, we don't care where
//if the virtual address < KERN_BASE_PAGE_NO == user page
void mmap(size_t base, size_t len, mmap_flags_t flags)
{
	assert((base & PGMASK) == 0); //Has to be 4k aligned
	//len = PG_ROUND_UP(len);
	for (size_t x = 0; x < len / PGSIZE; x++) {
		vptr_t vaddr = base + x * PGSIZE;
		if (is_mapped(current_directory, vaddr)) {
			if (!flags.IGNORE_PAGE_MAPPED) {
				print(LOG_WARNING,
				      "MMAP: Already mapped: 0x%x\n", base);
			}
			continue;
		}
		pptr_t phyaddr = alloc_frame();
		setPTE(current_directory, vaddr, phyaddr);
		if (flags.MAP_IMMEDIATELY) {
			invlpg(vaddr);
		}
	}
}

//Map it to a specific address, ex. for the kernel code or MMIO
void mmap_addr(vptr_t vaddr_start, pptr_t phyaddr_start, size_t len,
	       mmap_flags_t flags)
{
	assert((vaddr_start & PGMASK) == 0); //Has to be 4k aligned
	assert((phyaddr_start & PGMASK) == 0); //Has to be 4k aligned
	assert((len & PGMASK) == 0); //Len has to be multiples of PGSIZE

	if (is_mapped(current_directory, vaddr_start)) {
		if (!flags.IGNORE_PAGE_MAPPED) {
			print(LOG_ERROR, "MMAP_ADDR: Already mapped: 0x%x\n",
			      vaddr_start);
		}
		return;
	}

	//Mark all the frames as used first and then actually create the entries
	//This ensures the pages are contiguous
	//Also, at boot time, it makes sure the kernel code is marked as used, then the
	//page table frames are allocated

	//Mark pages used
	for (size_t x = 0; x < len / PGSIZE; x++) {
		pptr_t phyaddr = phyaddr_start + x * PGSIZE;

		frame_set_used(phyaddr, flags.IGNORE_FRAME_REUSE);
	}

	//Make the page entries
	for (size_t x = 0; x < len / PGSIZE; x++) {
		vptr_t vaddr = vaddr_start + x * PGSIZE;
		pptr_t phyaddr = phyaddr_start + x * PGSIZE;
		setPTE(current_directory, vaddr, phyaddr);
		if (flags.MAP_IMMEDIATELY) {
			invlpg(vaddr);
		}
	}
}

void setPTE(page_directory_t *pgdir, vptr_t vaddr, pptr_t phyaddr)
{
	uint32_t pdx = PDX(vaddr);
	uint32_t ptx = PTX(vaddr);

	//Page table does not exist yet so create it
	if (!(pgdir->actual_tables[pdx].present)) {
		vptr_t page = (vptr_t)kvmalloc(PGSIZE);
		memset((void *)page, 0, PGSIZE); //Initialize table

		//Set the pointer in the page directory table
		pgdir->tables[pdx] = (page_table_t *)page;
		pptr_t frame_addr = virtual_to_physical(pgdir, page);

		page_dir_entry_t *pde = &pgdir->actual_tables[pdx];
		if (pdx < KERN_BASE_PAGE_NO) {
			pde->user = 1;
		}
		pde->rw = 1;
		pde->frame = frame_addr >> POFFSHIFT;
		pde->present = 1;
	}
	pte_t *page_entry = &pgdir->tables[pdx]->pages[ptx];

	assert(!page_entry->present); //Entry not already mapped

	page_entry->rw = 1;
	page_entry->frame = PTE_ADDR(phyaddr); //Set the frame address
	page_entry->user = pdx < KERN_BASE_PAGE_NO;
	page_entry->present = 1;
}

void paging_init(size_t memory_map_base, size_t memory_map_full_len)
{
	frame_init(memory_map_base, memory_map_full_len);
	register_isr_handler(TRAP_PAGE_FAULT, page_fault_handler);

	size_t kernel_end_phy_addr = (size_t)&_kernel_end - KERN_BASE;

	//Mark kernel in use
	for (uint32_t i = 0; i < kernel_end_phy_addr; i += PGSIZE) {
		frame_set_used(i, FLAG_IGNORE_PHY_REUSE);
	}
	//Mark initial 4MB heap in use
	for (uint32_t i = kernel_end_phy_addr;
	     i <= kernel_end_phy_addr + LPGSIZE; i += PGSIZE) {
		frame_set_used(i, FLAG_IGNORE_PHY_REUSE);
	}

	current_directory =
		(page_directory_t *)kvmalloc(sizeof(page_directory_t));
	memset(current_directory, 0, sizeof(page_directory_t));

	kernel_page_directory = current_directory;

	mmap_flags_t flags = { .IGNORE_FRAME_REUSE = 1 };
	//Map kernel code
	mmap_addr(KERN_BASE, 0x0, kernel_end_phy_addr, flags);

	//First 4MB of heap are mapped already by the boot_page_directory
	flags.bits = 0;
	flags.IGNORE_FRAME_REUSE = 1;
	mmap_addr(KERN_HEAP_START, kernel_end_phy_addr, 0x400000, flags);

	//Map the rest of it
	flags.bits = 0;
	mmap(KERN_HEAP_START + 0x400000,
	     KERN_HEAP_END - KERN_HEAP_START - 0x400000, flags);

	switch_page_directory(current_directory); //Load the new page directory
	DisablePSE(); //Disable 4 MiB pages

	PAGING_INIT = 1;
}
static inline uint32_t read_cr2()
{
	uint32_t ret;
	asm("movl %%cr2, %0" : "=r"(ret));
	return ret;
}
void page_fault_handler(int_regs_t *regs)
{
	static char *page_fault_msgs[] = {
		"Supervisory process tried to read a non-present page entry",
		"Supervisory process tried to read a page and caused a protection fault",
		"Supervisory process tried to write to a non-present page entry",
		"Supervisory process tried to write a page and caused a protection fault",
		"User process tried to read a non-present page entry",
		"User process tried to read a page and caused a protection fault",
		"User process tried to write to a non-present page entry",
		"User process tried to write a page and caused a protection fault"
	};
	uint32_t cr2 = read_cr2();
	debug_print("Page Fault Error: %s at 0x%x\n",
		    page_fault_msgs[regs->error_code], cr2);
	halt();
}
void alloc_page(pte_t *page, int is_user, int is_writable)
{
	page->user = is_user;
	page->rw = is_writable;

	if (page->frame != 0) {
		page->present = 1;
		return;
	}
	pptr_t frame = alloc_frame();
	page->frame = frame >> 12;
	page->present = 1;
}

page_table_t *clone_table(page_table_t *src)
{
	page_table_t *dst = kvmalloc(sizeof(page_table_t));
	memset(dst, 0, sizeof(page_table_t));

	for (int x = 0; x < 1024; x++) {
		pte_t pg = src->pages[x];
		if (pg.present == 0) {
			continue;
		}

		pptr_t oldFrame = FRAME_TO_ADDR(pg.frame);
		pptr_t newFrame = alloc_frame();

		memcpy_frame_contents(newFrame, oldFrame);

		pg.frame = ADDR_TO_FRAME(newFrame);

		dst->pages[x] = pg;
	}

	return dst;
}
/*
 * Copies the page directory
 * Keeps the kernel mappings, but copies the user entries/tables
*/
page_directory_t *clone_directory(page_directory_t *src_pg_dir)
{
	page_directory_t *new_pg_directory = kvmalloc(sizeof(page_directory_t));
	memset(new_pg_directory, 0, sizeof(page_directory_t));

	for (uint32_t x = 0; x < 1024; x++) {
		page_dir_entry_t entry_bits = src_pg_dir->actual_tables[x];
		page_table_t *src_tbl = src_pg_dir->tables[x];

		//Keep kernel tables the same
		if (x >= KERN_BASE_PAGE_NO) {
			new_pg_directory->actual_tables[x] =
				src_pg_dir->actual_tables[x];
			new_pg_directory->tables[x] = src_pg_dir->tables[x];
			continue;
		}

		if (entry_bits.bits != 0) {
			page_table_t *dst_tbl = clone_table(src_tbl);
			pptr_t dst_tbl_phy = virtual_to_physical(
				current_directory, (vptr_t)dst_tbl);

			entry_bits.frame = ADDR_TO_FRAME(dst_tbl_phy);

			//Set the entry for the cloned table
			new_pg_directory->actual_tables[x] = entry_bits;
			new_pg_directory->tables[x] = dst_tbl;
		}
	}

	return new_pg_directory;
}
void free_table(page_table_t *src_tbl)
{
	for (int x = 0; x < 1024; x++) {
		if (src_tbl->pages[x].frame != 0) {
			free_frame(FRAME_TO_ADDR(src_tbl->pages[x].frame));
			src_tbl->pages[x].frame = 0;
			src_tbl->pages[x].present = false;
		}
	}
	kfree(src_tbl);
}
void free_user_mappings(page_directory_t *src_pg_dir)
{
	for (uint32_t x = 0; x < 1024; x++) {
		page_dir_entry_t entry_bits = src_pg_dir->actual_tables[x];
		page_table_t *src_tbl = src_pg_dir->tables[x];

		if (entry_bits.bits != 0 && x < KERN_BASE_PAGE_NO) {
			free_table(src_tbl);
			entry_bits.bits = 0;
		}
	}
}
void free_page_directory(page_directory_t *src_pg_dir)
{
	free_user_mappings(src_pg_dir);
	kfree(src_pg_dir);
}

void clear_pte(vptr_t addr)
{
	uint32_t pdx = PDX(addr);
	uint32_t ptx = PTX(addr);
	if (current_directory->tables[pdx] != NULL) {
		current_directory->tables[pdx]->pages[ptx].present = 0;
	}
}
void memcpy_frame_contents(pptr_t dst, pptr_t src)
{
	//Unmap pages
	clear_pte(COPY_PAGE_SOURCE);
	clear_pte(COPY_PAGE_DEST);

	//Map these two pages
	setPTE(current_directory, COPY_PAGE_SOURCE,
	       src); //TODO PAGE_R | PAGE_KERNEL
	setPTE(current_directory, COPY_PAGE_DEST,
	       dst); //TODO PAGE_RW | PAGE_KERNEL

	//invalidate entries
	invlpg(COPY_PAGE_SOURCE);
	invlpg(COPY_PAGE_DEST);

	//copy
	memcpy((void *)COPY_PAGE_DEST, (void *)COPY_PAGE_SOURCE, PGSIZE);

	//TODO
	//remove mappings
	//invalidate entries
}

//Graphic showing memory layout, modified from https://pdos.csail.mit.edu/6.828/2008/lec/l5.html
/*
        4 Gig -------->  +------------------------------+
                        :              .               :
                        :              .               :
                        :              .               :
                        |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~| 0xE0F00000
                        |   Remapped IO                | RW/--
                        |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~| 0xE0002000
                        | Temporary Copy Page (dest)   | RW/--
  COPY_PAGE_DEST  -->   |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~| 0xE0001000
                        | Temporary Copy Page (source) | RW/--
  COPY_PAGE_SOURCE -->  |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~| 0xE0000000
                        |                              | RW/--
                        |   Kernel Heap                | RW/--
                        |                              | RW/--
  KERN_HEAP_START  -->  +------------------------------+ 0xD0000000
                        |                              | RW/--
                        |   Remapped Physical Memory   | RW/--
                        |                              | RW/--
        KERNBASE -----> +------------------------------+ 0xc0000000
                        |       Empty Memory           | R-/R-  PTSIZE
        UPAGES    ----> +------------------------------+ 
                        |           RO ENVS            | R-/R-  PTSIZE
     UTOP,UENVS ------> +------------------------------+ 
    UXSTACKTOP -/       |     User Exception Stack     | RW/RW  PGSIZE
                        +------------------------------+ 
                        |       Empty Memory           | --/--  PGSIZE
        USTACKTOP  ---> +------------------------------+ 0xB0000000 
                        |      Normal User Stack       | RW/RW  PGSIZE
						+------------------------------+
						|       Empty Memory           |
	   USTACKTOP2  ---> +------------------------------+ 0xAFFFE000 
                        |      Thread 1 stack          | RW/RW  PGSIZE
                        +------------------------------+
                        |                              |
                        |                              |
                        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        .                              .
                        .                              .
                        .                              .
                        |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
                        |     Program Data & Heap      |
        UTEXT -------->  +------------------------------+ 0x00800000
        PFTEMP ------->  |       Empty Memory           |        PTSIZE
                        |                              |
        UTEMP -------->  +------------------------------+ 0x00400000
                        |       Empty Memory           |        PTSIZE
        0 ------------>  +------------------------------+
*/