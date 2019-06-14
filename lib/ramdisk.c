#include "sys/types.h"
#include "kmalloc.h"
#include "device.h"

typedef struct initrd_struct {
	uint8_t *ramdisk;
	uint32_t size;
	dev_t dev;
} initrd_t;

initrd_t initrd;

static int read_block(uint8_t *dst, uint32_t block_no);
static int write_block(uint8_t *src, uint32_t block_no);

void initrd_init(size_t start, size_t size)
{
	initrd.ramdisk = (uint8_t *)start;
	initrd.size = size;
	initrd.dev = 0x1000;
	device_register(BLOCK_DEVICE, initrd.dev, (void *)read_block,
			(void *)write_block, NULL);
}

static int read_block(uint8_t *dst, uint32_t block_no)
{
	if (block_no * BLOCK_SIZE > initrd.size) {
		return -1;
	}
	uint8_t *block = initrd.ramdisk + (block_no * BLOCK_SIZE);

	memcpy(dst, block, BLOCK_SIZE);
	return BLOCK_SIZE;
}
static int write_block(uint8_t *src, uint32_t block_no)
{
	if (block_no * BLOCK_SIZE > initrd.size) {
		return -1;
	}
	uint8_t *block = initrd.ramdisk + (block_no * BLOCK_SIZE);
	memcpy(block, src, BLOCK_SIZE);
	return BLOCK_SIZE;
}
