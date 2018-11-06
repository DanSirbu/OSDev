#pragma once
#include "types.h"

#define FILE_DEVICE 0
#define BLOCK_DEVICE 1

#define BLOCK_SIZE 512

typedef uint16_t dev_t;

void device_register(uint8_t dev_type, dev_t dev, void *read_func,
		     void *write_func, void *ioctl);

int device_read(dev_t dev, void *buf, uint32_t block);

int device_write(dev_t dev, void *buf, uint32_t block);