#include "device.h"
#include "serial.h"

#define MAX_DEVICES 10

typedef int (*block_read_func) (void *buf, uint32_t block);
typedef int (*block_write_func) (void *buf, uint32_t block);

typedef struct block_device {
	dev_t dev;
	block_read_func read;
	block_write_func write;
	char pad[1]; //So its the same size as a char_device
} block_dev_t;

block_dev_t block_devices[MAX_DEVICES];
uint32_t lastBlockIndex = 0;

void device_register(uint8_t dev_type, dev_t dev, void *read_func,
		     void *write_func, void *ioctl)
{
	if (dev_type == FILE_DEVICE) {
	} else if (dev_type == BLOCK_DEVICE) {
		block_dev_t *block_dev = &block_devices[lastBlockIndex++];
		block_dev->dev = dev;
		block_dev->read = read_func;
		block_dev->write = write_func;
	} else {
		fail("device_register: Unknown device type");
	}
}
int device_read(dev_t dev, void *buf, uint32_t block)
{
	for (int x = 0; x < MAX_DEVICES; x++) {
		if (block_devices[x].dev == dev)
			return block_devices[x].read(buf, block);
	}
	return -1;
}
int device_write(dev_t dev, void *buf, uint32_t block)
{
	for (int x = 0; x < MAX_DEVICES; x++) {
		if (block_devices[x].dev == dev)
			return block_devices[x].write(buf, block);
	}
	return -1;
}