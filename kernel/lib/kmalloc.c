#include "kmalloc.h"
#include "serial.h"
#include "sys/types.h"
#include "debug.h"

void sbrk_alignto(size_t alignment);

// Block header is 8 bytes total
typedef struct block {
	size_t size;
	struct block *next_free;
} __attribute__((packed)) block_t;

#define B2P(mblock) ((block_t *)mblock + 1)
#define P2B(mblock) (((block_t *)mblock) - 1)

block_t *free_list;

size_t heap_top;
size_t heap_start;
size_t heap_end;
/*
 * Can be called multiple times but the second time only updates the heap_end if it's bigger
 */
void kinit_malloc(size_t start, size_t end)
{
	if (heap_top) { //Already initialized
		if (heap_end < end) {
			debug_print(
				"Before heap expansion, there are 0x%x bytes left\n",
				heap_end - heap_top);
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
	}
	cur_block->next_free = NULL;
}
static inline void mark_block_free(block_t *block)
{
	if (free_list != NULL) {
		block->next_free = free_list;
	} else {
		block->next_free = NULL;
	}
	free_list = block;
}
void *kmalloc_align(size_t size, uint8_t alignment)
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
		if (current->size == 4096 &&
		    ((size_t)B2P(current) & 0xFFF) == 0) {
			mark_block_used(prev, current);
			return B2P(current);
		}
	}

	//We want to first align it to 4096 - block_t header size to make sure there is enough space
	//Note: VERY WASTEFUL, change this soon
	sbrk_alignto(4096 - sizeof(block_t));

	block_t *cur_block = (block_t *)sbrk(sizeof(block_t) + size);
	cur_block->next_free = 0;
	cur_block->size = size;
	return B2P(cur_block);
}
void *kmalloc(size_t size)
{
	if (size == 0) {
		return NULL;
	}
	if (!heap_start) {
		debug_print(
			"ERROR: Trying to malloc before the heap is initialized.");
		assert(1 == 2);
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
void *kcalloc(size_t size)
{
	void *ptr = kmalloc(size);
	memset(ptr, 0, size);
	return ptr;
}
void *krealloc(void *ptr, size_t newSize)
{
	if (newSize == 0 && ptr != NULL) {
		kfree(ptr);
		return NULL;
	}
	void *new_ptr = kmalloc(newSize);
	if (new_ptr == NULL) {
		return NULL;
	}
	if (ptr == NULL) {
		return new_ptr;
	}
	block_t *old_block = P2B(ptr);
	memcpy(new_ptr, ptr, MIN(old_block->size, newSize));
	kfree(ptr);
	return new_ptr;
}
void kfree(void *ptr)
{
	if (ptr == NULL) {
		return;
	}
	if (ptr < (void *)heap_start) {
		print(LOG_ERROR,
		      "Trying to free %p, which is before the heap_start (%p)\n",
		      ptr, (void *)heap_start);
		assert(ptr >= (void *)heap_start);
		return;
	}

	block_t *cur_block = P2B(ptr);

	// If the block is at the end of the heap, simply remove it and don't put in
	// free_list
	if ((ptr + cur_block->size) == (void *)heap_top) {
		sbrk(-(sizeof(block_t) + cur_block->size));
	} else { //Append to free list
		mark_block_free(cur_block);
	}
}
void kfree_arr(char **ptr1)
{
	char **ptr = (char **)ptr1;

	int i = 0;
	while (ptr[i] != NULL) {
		kfree(ptr[i]);
		i++;
	}

	kfree(ptr);
}

/*
 * NOTE: VERY WASTEFULL, this memory is never reclaimed
 * TODO, change allocator type
*/
void sbrk_alignto(size_t alignment)
{
	size_t addr = (size_t)sbrk(0);
	size_t curOffset = (size_t)addr & 0xFFF;

	if (alignment == curOffset) {
		return;
	} else if (curOffset < alignment) {
		sbrk(alignment - curOffset);
	} else {
		sbrk(alignment - (curOffset - alignment));
	}
}
void *sbrk(ssize_t size)
{
	void *returnVal = (void *)heap_top;
	heap_top += size;

	assert(heap_top <= heap_end);
	assert(heap_top >= heap_start); // Should never happen

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
	void *tempLocation = kmalloc(n);
	memcpy(tempLocation, src, n);
	memcpy(dest, tempLocation, n);
	kfree(tempLocation);

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
