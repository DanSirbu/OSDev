#include "types.h"
#include "p_allocator.h"
#include "mmu.h"
#include "serial.h"
#include "trap.h"

extern void LoadNewPageDirectory(uint32_t pd);
extern void DisablePSE();
extern size_t boot_page_directory[1024];
extern char kernel_end;

void page_fault_handler(int error_no);

uint32_t page_directory[1024] __attribute__((aligned(0x1000)));

pptr_t virtual_to_physical(vptr_t addr)
{
	uint32_t pdx = PDX((size_t)addr);
	uint32_t ptx = PTX((size_t)addr);

	page_directory_t *page_table =
		(page_directory_t *)(0xFFC00000 | pdx << 12);
	return (pptr_t)((page_table->tables[ptx] & ~0xFFF) |
			((size_t)addr & 0xFFF));
}

//Just map it, we don't care where
void mmap(size_t base, size_t len)
{
	assert((base & PGSIZE) == 0); //Has to be 4k aligned
	//len = PG_ROUND_UP(len);
	for (size_t x = 0; x < len / PGSIZE; x++) {
		size_t vaddr = base + x * PGSIZE;
		size_t phyaddr = alloc_frame();
		setPTE(vaddr, phyaddr);
	}
}

//Map it to a specific address, ex. for the kernel code or MMIO
void mmap_addr(size_t vaddr_start, size_t phyaddr_start, size_t len,
	       uint8_t flags)
{
	assert((vaddr_start & 0xFFF) == 0); //Has to be 4k aligned
	assert((phyaddr_start & 0xFFF) == 0); //Has to be 4k aligned

	size_t vaddr_pdx = PDX(vaddr_start);
	size_t vaddr_ptx = PTX(vaddr_start);

	//Mark all the pages as used first and then actually create the entries
	//This ensures the pages are contiguous
	//Also, at boot time, it makes sure the kernel code is marked as used, then the
	//page tables are allocated

	//Mark pages used
	for (size_t x = 0; x < len / PGSIZE; x++) {
		size_t vaddr = vaddr_start + x * PGSIZE;
		ptr_phy_t phyaddr = phyaddr_start + x * PGSIZE;

		frame_set_used(phyaddr, flags & FLAG_IGNORE_PHY_REUSE);
	}

	//Make the page entries
	for (size_t x = 0; x < len / PGSIZE; x++) {
		size_t vaddr = vaddr_start + x * PGSIZE;
		ptr_phy_t phyaddr = phyaddr_start + x * PGSIZE;
		setPTE(vaddr, phyaddr);
	}
}
void setPTE(size_t vaddr, ptr_phy_t phyaddr)
{
	uint32_t pdx = PDX(vaddr);
	uint32_t ptx = PTX(vaddr);

	//Page table does not exist yet so create it
	if (!(page_directory[pdx] & PTE_P)) {
		ptr_phy_t pg_tbl_phyaddr = alloc_frame();
		page_directory[pdx] = PTE_ADDR(pg_tbl_phyaddr) | PTE_P;

		//Clear data so page table is initialized
		//Because the last page directory entry is mapped to itself
		//We can access the page table this way
		size_t *pg_tbl = (size_t *)((0xFFC << 20) | (pdx << 12));
		memset((char *)pg_tbl, 0, PGSIZE);
	}
	//Because the last page directory entry is mapped to itself
	//We can access the page table this way
	size_t *pg_tbl = (size_t *)((0xFFC << 20) | (pdx << 12));

	assert(!(pg_tbl[ptx] & PTE_P)); //Entry not already mapped

	pg_tbl[ptx] = PTE_ADDR(phyaddr) | PTE_P; //Set the frame address
}

void paging_finalize();
void paging_init(size_t memory_map_base, size_t memory_map_full_len)
{
	frame_init(memory_map_base, memory_map_full_len);
	register_handler(TRAP_PAGE_FAULT, page_fault_handler);

	//We need the last page directory entry to map to page directory
	// so we can map the page tables to memory
	boot_page_directory[1023] =
		PTE_ADDR((size_t)page_directory - KERN_BASE) | PTE_P; //4kb page
	//Invalidate tlb
	//TODO: Look into using invlpg
	LoadNewPageDirectory((uint32_t)boot_page_directory - KERN_BASE);

	//Clear page directory
	memset(page_directory, 0, sizeof(page_directory));

	//Add recursive mapping
	page_directory[1023] =
		PTE_ADDR((size_t)page_directory - KERN_BASE) | PTE_P;

	//Now we need to mark the IO as used
	//We map all IO to KERN_IO_BASE - size of all IO
	/*void *memory_map = (void *)memory_map_base + KERN_BASE;
	void *memory_map_end = memory_map + memory_map_full_len;

	size_t io_cur_location = KERN_IO_BASE;
	for (; memory_map < memory_map_end;
	     memory_map +=
	     ((memory_map_t *)memory_map)->size + sizeof(unsigned long)) {
		memory_map_t *mmap_cur = (memory_map_t *)memory_map;

		if (mmap_cur->type != 2) {
			continue;
		}

		uint8_t map_flag =
			mmap_cur->base_addr_low < kernel_end_phy_addr ?
				FLAG_IGNORE_PHY_REUSE :
				NULL;

		mmap_addr(io_cur_location,
			  PG_ROUND_DOWN(mmap_cur->base_addr_low),
			  PG_ROUND_UP(mmap_cur->length_low), map_flag);

		io_cur_location += PG_ROUND_UP(mmap_cur->length_low);
	}*/
	paging_finalize();
	//Load new page directory
	LoadNewPageDirectory((uint32_t) virtual_to_physical(page_directory));

	//Disable PSE (4 MiB pages)
	DisablePSE();
}
void paging_finalize() {
	size_t kernel_end_phy_addr =
		PG_ROUND_UP((size_t)&kernel_end - KERN_BASE);

	//Map kernel code
	mmap_addr(KERN_BASE, 0x0, kernel_end_phy_addr, FLAG_IGNORE_PHY_REUSE);
	//Map heap
	mmap(KERN_HEAP_START, KERN_HEAP_END - KERN_HEAP_START);
}
void page_fault_handler(int error_no)
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
	fail_stmt_stop("Page Fault Error: %s\n", page_fault_msgs[error_no]);
}

/*
 * Copies the page directory, all the user page tables and the contents of the user page tables
*/
page_directory_t *clone_directory(page_directory_t *src)
{
	/*	void *new_pg_directory = kmalloc(sizeof(page_directory_t));

	for (int x = 0; x < 1024; x++) {
		if (src->tables[x] != 0) {
		}

		//Keep kernel tables the same
		if (x > (KERN_BASE / PGSIZE)) {
			new_pg_directory->tables[x] = pd->tables[x];
		}
	}
	*/
	return src; //TODO
}