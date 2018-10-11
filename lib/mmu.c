#include "../include/types.h"

extern void LoadNewPageDirectory(uint32_t pd);

uint32_t page_directory[1024]; //at addr 0xc0103000
									     //0x103000

void setPTE(uint32_t source, uint32_t target) {
	//0xC0000000 -> 0x0
	uint32_t pte_index = source >> 22; //source / 4MB

	uint32_t pte_entry = 0x83; //0x83 = 4mb page size, R/W, Present
	pte_entry |= target & ~0xFFF;

	page_directory[pte_index] = pte_entry;
}

void paging_init() {
	/*
	; Map 0xFEBC0000 -> 0xFEBC0000
	ETHERNET_BASE equ 0xFEBC0000
	ETHERNET_PAGE_NUM equ ETHERNET_BASE >> 22

	Map 0xC0000000 -> 0x0 for our kernel

	*/

	for(int x = 0; x < 1024; x++) {
		page_directory[x] = 0;
	}
	setPTE(0xC0000000, 0x0);
	setPTE(0xFEBC0000, 0xfe800000); //ETHERNET_BASE equ 0xFEBC0000
    //0xfe800000->0xfec00000

	uint32_t pd_target = ((uint32_t) page_directory) - 0xC0000000;
	LoadNewPageDirectory(pd_target);
}
struct PAGE_DIRECTORY_ENTRY {
	unsigned char present: 1;
	unsigned char writable: 1;
	unsigned char user: 1;
	unsigned char write_through : 1;
	unsigned char cache_disabled : 1;
	unsigned char accessed : 1;
    unsigned char zero : 1;
	unsigned char size_4mb : 1;
	unsigned char g: 1;
	unsigned char avail: 3;
	unsigned int address: 20;
};