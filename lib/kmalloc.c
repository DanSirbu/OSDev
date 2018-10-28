#include "../include/kmalloc.h"
#include "../include/serial.h"
#include "../include/types.h"

extern char kernel_end;

struct block_header {
	size_t size;
	struct block_header *next_free;
} __attribute__((packed)); // Block header is 8 bytes total
typedef struct block_header block;

#define B2P(mblock) (mblock + 1)
#define P2B(mblock) (((block *)mblock) - 1)

block *free_list;
void *heap_head = (void *)0x00800000;
void *heap_max = (void *)0x00c00000;
void *heap_top = (void *)0x00800000;
// kpanic_fmt("kmalloc 0x%x\n", (u64) (u32) (void *) &kernel_end);

void *kmalloc_align(size_t size, uint8_t alignment)
{
	if (size == 0) {
		return NULL;
	}
	// For now to keep it simple, just allocate new space, don't use previous
	// space

	if (alignment < 8) {
		kpanic_fmt(
			"KMALLOC_ALIGNED NOT IMPLEMENTED WITH smaller than 8");
	}
	// TODO make this code thread safe
	uint32_t curAlignment = (uint32_t)sbrk(0);
	uint32_t wantedAlignment = curAlignment; // 8 bits aligned
	ALIGN(wantedAlignment, 8);
	wantedAlignment += 8;
	ALIGN(wantedAlignment, alignment);
	// Now we know wanted alignment has at least 8 bytes free before it and is
	// currently aligned to "alignment"
	sbrk((wantedAlignment - 8) - curAlignment);

	ALIGN(size, 8);
	block *cur_block = (block *)sbrk(sizeof(block) + size);
	cur_block->next_free = 0;
	cur_block->size = size;
	return B2P(cur_block);
}
void *kmalloc(size_t size)
{
	if (size == 0) {
		return NULL;
	}
	// Make size 8 byte aligned
	ALIGN(size, 8);
	// Find best fit for size while keeping track of smallest fit
	block *curBestFit = NULL, *curBestFitPrevious = NULL;
	block *prev_block, *cur_block;
	
	for (cur_block = free_list, prev_block = free_list; cur_block != NULL;
	     prev_block = cur_block, cur_block = cur_block->next_free) {
		if (cur_block->size < size)
			continue;

		if (cur_block->size == size) { // Found our perfect match
			prev_block->next_free =
				cur_block->next_free; // Remove block from free_list
			cur_block->next_free =
				NULL; // Remove pointer since it's not free anymore
			return B2P(cur_block);
		}

		if (cur_block->size <
		    curBestFit->size) { // The current block is the smallest that fits the data
			curBestFit = cur_block;
			curBestFitPrevious = prev_block;
		}
	}

	// Use the smallest fit even if not ideal
	if (curBestFit != NULL) {
		curBestFitPrevious->next_free =
			cur_block->next_free; // Remove block from free_list
		curBestFit->next_free =
			NULL; // Remove pointer since it's not free anymore
		return B2P(curBestFit);
	}

	// Else must allocate space
	cur_block = (block *)sbrk(sizeof(block) + size);
	cur_block->next_free = 0;
	cur_block->size = size;
	return B2P(cur_block);
}

void kfree(void *ptr)
{
	if (ptr == 0) {
		return;
	}
	if (ptr < heap_head) {
		kpanic_fmt(
			"Trying to free ptr before the head of the heap. %p\n",
			ptr);
		return;
	}

	block *cur_block = P2B(ptr);

	// If the block is at the end of the heap, simply remove it and don't put in
	// free_list
	if ((ptr + cur_block->size) == heap_top) {
		sbrk(-(sizeof(block) + cur_block->size));
		return;
	}

	if (free_list == NULL) { // The first block we freed
		free_list = cur_block;
		return;
	}

	// Else we append to free list
	block *cur_free_block;
	for (cur_free_block = free_list; cur_free_block->next_free != NULL;
	     cur_free_block = cur_free_block->next_free)
		;
	// We are at the last block, so append ours here
	cur_free_block->next_free = cur_block;
}

void *sbrk(u32 size)
{
	if (size == 0) {
		return heap_top;
	}
	void *returnVal = heap_top;
	heap_top += size;
	if (heap_top > heap_max) {
		kpanic_fmt("HEAP TOP OVER HEAP MAX\n");
	}

	// Should never happen
	if (heap_top < heap_head) {
		kpanic_fmt("PANIC: heap top < heap head\n");
	}
	return returnVal;
}

void memset(char *ptr, uint8_t value, size_t s)
{
	for (size_t x = 0; x < s; x++) {
		ptr[x] = value;
	}
}
void memcpy(char *dst, char *src, size_t s)
{
	for (size_t x = 0; x < s; x++) {
		dst[x] = src[x];
	}
}

uint32_t physical_to_virtual(uint32_t phys)
{
	return phys + KERN_BASE;
}