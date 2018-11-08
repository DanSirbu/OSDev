#include "p_allocator.h"
#include "kmalloc.h"
#include "serial.h"

//Comes from the linker
extern char kernel_end;

//Use a bitmap like the james malloy tutorial. TODO, change to how xv6 does it
//Because this is inefficient, especially at initialization
#define PGSIZE 4096
//Use 1 bit to show freed frame
//32 bits of memory / PGSIZE = 0xFFFFF000
//0xFFFFF / uint32 (32) = 0xFFFE0 = 1048544 entries (1 mb)
#define BIT_FRAME_SIZE (PGSIZE * 32)
#define NUM_BIT_FRAMES 0xFFFE0

uint32_t frames[NUM_BIT_FRAMES] __attribute__((aligned(32)));

void frame_init(size_t memory_map_base, size_t memory_map_len)
{
	//Set all used, then free the ones that are actually usable
	memset((void *)frames, 0xFF,
	       NUM_BIT_FRAMES * 4); //* 4 since 32 bits = 4 bytes

	void *memory_map = (void *)memory_map_base + KERN_BASE;
	void *memory_map_end = memory_map + memory_map_len;

	//Keep < kernel_end as used to not overwrite kernel stuff
	size_t kernel_end_phy_addr =
		PG_ROUND_UP((size_t)&kernel_end - KERN_BASE);

	for (; memory_map < memory_map_end;
	     memory_map +=
	     ((memory_map_t *)memory_map)->size + sizeof(unsigned long)) {
		memory_map_t *mmap_cur = (memory_map_t *)memory_map;

		size_t base_addr = PG_ROUND_UP(mmap_cur->base_addr_low);
		size_t base_len = PG_ROUND_DOWN(mmap_cur->length_low);

		if (base_addr < kernel_end_phy_addr) {
			size_t substract_len = kernel_end_phy_addr - base_addr;

			//base_addr + base_len < kernel_end_addr
			if (substract_len > base_len) {
				base_len = 0; //So ignore this memory region
			} else {
				base_addr = kernel_end_phy_addr;
				base_len -= substract_len;
			}
		}

		free_frame_range(base_addr, base_len);
	}
}

ptr_phy_t alloc_frame()
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
	kpanic_fmt("No more free frames\n");
	return 0xFFFFFFFF;
}
void frame_set_used(ptr_phy_t frame, uint8_t reuse_ok)
{
	uint32_t frame_index = frame / BIT_FRAME_SIZE;
	uint32_t frame_shift = 1U << ((frame % BIT_FRAME_SIZE) / PGSIZE);

	if ((frames[frame_index] & frame_shift) == 0) {
		frames[frame_index] |= frame_shift;
	} else if (!reuse_ok) {
		kpanic_fmt("Error: frame already used 0x%x\n", frame);
	}
}
void free_frame(ptr_phy_t frame)
{
	uint32_t frame_index = frame / BIT_FRAME_SIZE;
	uint32_t frame_shift = 1U << ((frame % BIT_FRAME_SIZE) / PGSIZE);

	if (frames[frame_index] & frame_shift) {
		frames[frame_index] &= ~frame_shift;
	} else {
		kpanic_fmt("Error: double free frame 0x%x\n", frame);
	}
}
void free_frame_range(ptr_phy_t first_frame, uint32_t len)
{
	for (uint32_t x = 0; x < len / PGSIZE; x++) {
		free_frame(first_frame + x * PGSIZE);
	}
}