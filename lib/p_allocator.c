#include "p_allocator.h"
#include "kmalloc.h"
#include "serial.h"

//Use a bitmap like the james malloy tutorial. TODO, change to how xv6 does it
//Because this is inefficient, especially at initialization

//Use 1 bit to show freed frame
//32 bits of memory 2^32 / PGSIZE 2^12 = 2^20 = 0x100000
//2^20 / unsigned int (32 bits) 2^5 = 2^15 = 0x8000
#define BIT_FRAME_SIZE 0x20000
#define NUM_BIT_FRAMES 0x8000

uint32_t frames[NUM_BIT_FRAMES] __attribute__((aligned(32)));
bool FRAME_MAP_INITIALIZED = 0;

void frame_init(size_t memory_map_base, size_t memory_map_len)
{
	//debug_print("Num: %x\n", (uint32_t)NUM_BIT_FRAMES);
	//Set all used, then free the ones that are actually usable
	memset(frames, 0xFF,
	       NUM_BIT_FRAMES * 4); //* 4 since 32 bits = 4 bytes

	void *memory_map = (void *)memory_map_base + KERN_BASE;
	void *memory_map_end = memory_map + memory_map_len;

	for (; memory_map < memory_map_end;
	     memory_map += ((multiboot_memory_map_t *)memory_map)->size +
			   sizeof(unsigned long)) {
		multiboot_memory_map_t *mmap_cur =
			(multiboot_memory_map_t *)memory_map;

		//Physical allocator only for non-io areas
		if (mmap_cur->type != 1) {
			continue;
		}

		size_t base_addr = PG_ROUND_UP(mmap_cur->addr);
		size_t base_len = PG_ROUND_DOWN(mmap_cur->len);

		free_frame_range(base_addr, base_len);
	}
	FRAME_MAP_INITIALIZED = 1;
}

pptr_t alloc_frame()
{
	for (int x = 0; x < 0xFFFE0; x++) {
		//There is a free frame here (since at least one bit is set to 0)
		if (frames[x] != 0xFFFFFFFF) {
			for (int i = 0; i < 32; i++) {
				//Free frame
				if ((frames[x] & (1U << i)) == 0) {
					frames[x] |= (1U << i); //Set used
					return x * BIT_FRAME_SIZE + i * PGSIZE;
				}
			}
		}
	}
	debug_print("No more free frames\n");
	return 0xFFFFFFFF;
}
void frame_set_used(pptr_t frame, uint8_t reuse_ok)
{
	uint32_t frame_index = frame / BIT_FRAME_SIZE;
	uint32_t frame_shift = 1U << ((frame % BIT_FRAME_SIZE) / PGSIZE);

	if ((frames[frame_index] & frame_shift) == 0) {
		frames[frame_index] |= frame_shift;
	} else if (!reuse_ok) {
		debug_print("Error: frame already used 0x%x\n", frame);
	}
}
void free_frame(pptr_t frame)
{
	uint32_t frame_index = frame / BIT_FRAME_SIZE;
	uint32_t frame_shift = 1U << ((frame % BIT_FRAME_SIZE) / PGSIZE);

	if (frames[frame_index] & frame_shift) {
		frames[frame_index] &= ~frame_shift;
	} else if (FRAME_MAP_INITIALIZED) { //TODO fix this hack, properly sort and merge memory map and remove FRAME_MAP_INITIALIZED variable
		debug_print("Error: double free frame 0x%x\n", frame);
		halt();
	}
}
void free_frame_range(pptr_t first_frame, uint32_t len)
{
	debug_print("Freeing frame ranges: base: %x len: %x\n", first_frame,
		    len);
	for (uint32_t x = 0; x < len / PGSIZE; x++) {
		free_frame(first_frame + x * PGSIZE);
	}
}