#include "../include/types.h"
#include "../include/p_allocator.h"
#include "../include/mmu.h"
#include "../include/serial.h"
#include "../include/trap.h"

extern void LoadNewPageDirectory(uint32_t pd);
extern void DisablePSE();
extern size_t boot_page_directory[1024];

void page_fault_handler(int error_no);

uint32_t page_directory[1024] __attribute__((aligned(0x1000)));

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
void mmap_addr(size_t vaddr, size_t phyaddr, size_t len)
{
	assert((vaddr & PGSIZE) == 0); //Has to be 4k aligned
	assert((phyaddr & PGSIZE) == 0); //Has to be 4k aligned

	size_t vaddr_pdx = PDX(vaddr);
	size_t vaddr_ptx = PTX(vaddr);

	//len = PG_ROUND_UP(len);
	for (size_t x = 0; x < len / PGSIZE; x++) {
		size_t cur_vaddr = vaddr + x * PGSIZE;
		ptr_phy_t cur_phyaddr = phyaddr + x * PGSIZE;
		setPTE(cur_vaddr, cur_phyaddr);
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

void paging_init()
{
	register_handler(TRAP_PAGE_FAULT, page_fault_handler);
	/*
  ; Map 0xFEBC0000 -> 0xFEBC0000
  ETHERNET_BASE equ 0xFEBC0000
  ETHERNET_PAGE_NUM equ ETHERNET_BASE >> 22

  Map 0xC0000000 -> 0x0 for our kernel

  */
	//We need the last page directory to map to page directory
	// so we can map the page tables to memory
	boot_page_directory[1023] =
		PTE_ADDR((size_t)page_directory - 0xC0000000) |
		PTE_P; //4kb page
	//Invalidate tlb
	//TODO: Look into using invlpg
	LoadNewPageDirectory((size_t)boot_page_directory - 0xC0000000);

	for (int x = 0; x < 1024; x++) {
		page_directory[x] = 0;
	}
	page_directory[1023] =
		PTE_ADDR((size_t)page_directory - 0xC0000000) | PTE_P;

	mmap_addr(0xC0000000, 0x0, 0x800000);

	// Identity mapping to use in malloc since we often need a physical address
	mmap_addr(0x00a00000, 0x00a00000, 0x400000);

	// ETHERNET_BASE equ 0xFEBC0000
	//mmap_addr(0xfe800000, 0xfe800000, 0x400000);
	//setPTE(0xcfc00000, 0x0fc00000); // Mapping for acpi

	uint32_t pd_target = ((uint32_t)page_directory) - 0xC0000000;
	LoadNewPageDirectory(pd_target);

	//Disable PSE (4 MiB pages)
	DisablePSE();
}
struct PAGE_DIRECTORY_ENTRY {
	unsigned char present : 1;
	unsigned char writable : 1;
	unsigned char user : 1;
	unsigned char write_through : 1;
	unsigned char cache_disabled : 1;
	unsigned char accessed : 1;
	unsigned char zero : 1;
	unsigned char size_4mb : 1;
	unsigned char g : 1;
	unsigned char avail : 3;
	unsigned int address : 20;
};

void page_fault_handler(int error_no)
{
	char *page_fault_msgs[] = {
		"Supervisory process tried to read a non-present page entry",
		"Supervisory process tried to read a page and caused a protection fault",
		"Supervisory process tried to write to a non-present page entry",
		"Supervisory process tried to write a page and caused a protection fault",
		"User process tried to read a non-present page entry",
		"User process tried to read a page and caused a protection fault",
		"User process tried to write to a non-present page entry",
		"User process tried to write a page and caused a protection fault"
	};
	kpanic_fmt("Page Fault Error: %s\n", page_fault_msgs[error_no]);
}