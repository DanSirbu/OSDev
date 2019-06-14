#pragma once
#include "sys/types.h"

typedef struct {
	size_t write_ptr;
	size_t read_ptr;
	size_t size;
	uint8_t *buffer;

	volatile int lock[2];
} ring_buffer_t;

uint32_t get_read_amount(ring_buffer_t *ring_buffer);
ring_buffer_t *create_ring_buffer(size_t size);
void free_ring_buffer(ring_buffer_t *ring_buffer);