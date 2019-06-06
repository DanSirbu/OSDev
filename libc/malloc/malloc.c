#include "assert.h"
#include "malloc.h"
#include "syscalls.h"

// Block header is 8 bytes total
typedef struct block {
	size_t size;
	struct block *next_free;
} __attribute__((packed)) block_t;

#define B2P(mblock) (mblock + 1)
#define P2B(mblock) (((block_t *)mblock) - 1)

block_t *free_list;

vptr_t heap_top = 0;
vptr_t heap_start = 0;
vptr_t heap_end = 0;

void *sbrk(uint32_t size);
void sbrk_alignto(size_t alignment);

/*
 * Can be called multiple times but the second time only updates the heap_end if it's bigger
 */
void kinit_malloc(vptr_t start, vptr_t end)
{
	if (heap_top) { //Already initialized
		if (heap_end < end) {
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
void *malloc_align(size_t size, uint8_t alignment)
{
	if (size == 0) {
		return NULL;
	}
	assert(alignment >= 8);

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
void *malloc(size_t size)
{
	if (size == 0) {
		return NULL;
	}

	// Make size 8 byte aligned
	ALIGN(size, 8);
	// Find best fit for size while keeping track of smallest fit and previous block
	block_t *curBestFit = NULL, *curBestFitPrevious = NULL;
	block_t *prev_block = NULL, *cur_block = NULL;

	for (cur_block = free_list; cur_block != NULL;
	     prev_block = cur_block, cur_block = cur_block->next_free) {
		if (cur_block->size < size)
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

	// Use the block that fits the best (least space wasted)
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
void *calloc(size_t size)
{
	void *ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}
void *realloc(void *ptr, size_t newSize)
{
	if (newSize == 0 && ptr != NULL) {
		free(ptr);
		return NULL;
	}
	void *new_ptr = malloc(newSize);
	if (new_ptr == NULL) {
		return NULL;
	}
	if (ptr == NULL) {
		return new_ptr;
	}
	block_t *old_block = P2B(ptr);
	memcpy(new_ptr, ptr, MIN(old_block->size, newSize));
	free(ptr);
	return new_ptr;
}
void free(void *ptr)
{
	if (ptr == NULL) {
		return;
	}
	assert(ptr >= (void *)heap_start);
	assert(ptr < (void *)heap_end);

	block_t *cur_block = P2B(ptr);

	// If the block is at the end of the heap, simply remove it and don't put in
	// free_list
	if ((ptr + cur_block->size) == (void *)heap_top) {
		sbrk(-(sizeof(block_t) + cur_block->size));
	} else { //Append to free list
		cur_block->next_free = free_list;
		free_list = cur_block;
	}
}
void free_arr(char **ptr1)
{
	char **ptr = (char **)ptr1;

	while (*ptr != NULL) {
		free(*ptr);
		ptr += 1;
	}

	free(ptr);
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
void *sbrk(uint32_t size)
{
	//Initialize heap if this is the first call to it
	if (heap_top == (vptr_t)NULL) {
		uint32_t alloc_size =
			0x200000; //TODO IMPORTANT make this value dynamic
		void *start = (void *)syscall_sbrk(alloc_size);
		kinit_malloc((vptr_t)start, (vptr_t)(start + alloc_size));
	}
	void *returnVal = (void *)heap_top;
	heap_top += size;

	assert(heap_top <= heap_end);
	assert(heap_top >= heap_start);

	return returnVal;
}

void *memset(void *ptr, int value, size_t s)
{
	for (size_t x = 0; x < s; x++) {
		((uint8_t *)ptr)[x] = (uint8_t)(value & 0xFF);
	}
	return ptr;
}
void *memcpy(void *dst, const void *src, size_t s)
{
	for (size_t x = 0; x < s; x++) {
		((uint8_t *)dst)[x] = ((uint8_t *)src)[x];
	}
	return dst;
}
void *memmove(void *dest, const void *src, size_t n)
{
	//TODO IMPORTANT, implement without malloc
	if (n == 0) {
		return dest;
	}
	void *tempLocation = malloc(n);
	memcpy(tempLocation, src, n);
	memcpy(dest, tempLocation, n);
	free(tempLocation);

	return dest;
}
int memcmp(const void *s1, const void *s2, size_t n)
{
	uint8_t *p1 = (uint8_t *)s1;
	uint8_t *p2 = (uint8_t *)s2;
	for (size_t x = 0; x < n; x++) {
		if (p2[x] != p1[x]) {
			return p2[x] - p1[x];
		}
	}
	return 0;
}