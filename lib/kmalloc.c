#include "kmalloc.h"
#include "serial.h"
#include "types.h"

void sbrk_alignto(size_t alignment);

// Block header is 8 bytes total
typedef struct block {
	size_t size;
	struct block *next_free;
} __attribute__((packed)) block_t;

#define B2P(mblock) (mblock + 1)
#define P2B(mblock) (((block_t *)mblock) - 1)

block_t *free_list;

vptr_t heap_top;
vptr_t heap_start;
vptr_t heap_end;
/*
 * Can be called multiple times but the second time only updates the heap_end if it's bigger
 */
void kinit_malloc(vptr_t start, vptr_t end)
{
	if (heap_top) { //Already initialized
		if(heap_end < end) {
			heap_end = end;
		}
		return;
	}

	heap_start = heap_top = start;
	heap_end = end;
}
/*
 * Remove a block from the free list
 * If the prev_block is null, then it means it's the first block
 */
static inline void mark_block_used(block_t *prev_block, block_t *cur_block)
{
	if (prev_block == NULL) {
		free_list = NULL;
	} else {
		prev_block->next_free = cur_block->next_free;
		cur_block->next_free = NULL;
	}
}
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
	block_t *cur_block = (block_t *)sbrk(sizeof(block_t) + size);
	cur_block->next_free = 0;
	cur_block->size = size;
	return B2P(cur_block);
}
/*
 * Allocate page-aligned block
 */
void *kvmalloc(size_t size)
{
	if (size == 0) {
		return NULL;
	}
	size = PG_ROUND_UP(size);
	block_t *current = NULL, *prev = NULL;

	for (current = free_list; current != NULL;
	     prev = current, current = current->next_free) {
		if (current->size == 4096) {
			mark_block_used(prev, current);
			return B2P(current);
		}
	}

	//TODO: allocate new page
	//We want to first align it to 4096 - block_t header size to make sure there is enough space
	sbrk_alignto(4096 - sizeof(block_t));
	return kmalloc(size);
}
void *kmalloc(size_t size)
{
	if (size == 0) {
		return NULL;
	}
	if(!heap_start) {
		kpanic_fmt("ERROR: Trying to malloc before the heap is initialized.");
		return NULL;
	}
	
	// Make size 8 byte aligned
	ALIGN(size, 8);
	// Find best fit for size while keeping track of smallest fit and previous block
	block_t *curBestFit = NULL, *curBestFitPrevious = NULL;
	block_t *prev_block = NULL, *cur_block = NULL;

	for (cur_block = free_list; cur_block != NULL;
	     prev_block = cur_block, cur_block = cur_block->next_free) {
		//Note: cur_block->size == 4096: Leave page-sized blocks alone for page directories
		if (cur_block->size < size || cur_block->size == 4096)
			continue;

		if (cur_block->size == size) { // Found our perfect match
			mark_block_used(prev_block, cur_block);
			return B2P(cur_block);
		}

		if (curBestFit == NULL ||
		    cur_block->size <
			    curBestFit->size) { // The current block is the smallest that fits the data
			curBestFit = cur_block;
			curBestFitPrevious = prev_block;
		}
	}

	// Use the smallest fit even if not ideal
	if (curBestFit != NULL) {
		mark_block_used(curBestFitPrevious, curBestFit);
		return B2P(curBestFit);
	}

	// Else must allocate space
	cur_block = (block_t *)sbrk(sizeof(block_t) + size);
	cur_block->next_free = 0;
	cur_block->size = size;
	return B2P(cur_block);
}

void kfree(void *ptr)
{
	if (ptr == 0) {
		return;
	}
	if (ptr < (void*)heap_start) {
		kpanic_fmt("Trying to free %p, which is before the heap_start (%p)\n", (void*)ptr, (void*)heap_start);
		return;
	}

	block_t *cur_block = P2B(ptr);

	// If the block is at the end of the heap, simply remove it and don't put in
	// free_list
	if ((ptr + cur_block->size) == (void*)heap_top) {
		sbrk(-(sizeof(block_t) + cur_block->size));
		return;
	}

	//Else append to free list
	cur_block->next_free = free_list;
	free_list = cur_block;
}

/*
 * NOTE: VERY WASTEFULL, this memory is never reclaimed
 * TODO, change allocator type
*/
void sbrk_alignto(size_t alignment)
{
	vptr_t addr = (vptr_t)sbrk(0);
	size_t curOffset = (size_t)addr & 0xFFF;

	if (alignment == curOffset) {
		return;
	} else if (curOffset < alignment) {
		sbrk(alignment - curOffset);
	} else {
		sbrk(alignment - (curOffset - alignment));
	}
}
void *sbrk(u32 size)
{
	void *returnVal = (void*)heap_top;
	heap_top += size;

	assert(heap_top <= heap_end);
	assert(heap_top >= heap_start); // Should never happen

	return returnVal;
}

void memset(void *ptr, char value, size_t s)
{
	for (size_t x = 0; x < s; x++) {
		((char *)ptr)[x] = value;
	}
}
void memcpy(void *dst, void *src, size_t s)
{
	for (size_t x = 0; x < s; x++) {
		((uint8_t *)dst)[x] = ((uint8_t *)src)[x];
	}
}