#include "types.h"
#include "kmalloc.h"
#include "ringbuffer.h"

ring_buffer_t *create_ring_buffer(size_t size)
{
	ring_buffer_t *ring_buffer = kmalloc(sizeof(ring_buffer_t));
	ring_buffer->read_ptr = ring_buffer->write_ptr = 0;

	ring_buffer->size = size;

	ring_buffer->buffer = kmalloc(size);

	return ring_buffer;
}
void free_ring_buffer(ring_buffer_t *ring_buffer)
{
	kfree(ring_buffer->buffer);
	kfree(ring_buffer);
}

void read_ring_buffer(ring_buffer_t *ring_buffer, size_t size, uint8_t *buf)
{
	uint32_t pos = ring_buffer->read_ptr;

	if (pos + size >= ring_buffer->write_ptr) {
		//sleep();
	} else {
		memcpy(buf, &ring_buffer->buffer[pos], size);
	}
}

uint32_t get_read_amount(ring_buffer_t *ring_buffer)
{
	if (ring_buffer->write_ptr < ring_buffer->read_ptr) {
		return ring_buffer->size - ring_buffer->read_ptr +
		       ring_buffer->write_ptr;
	} else {
		return ring_buffer->write_ptr - ring_buffer->read_ptr;
	}
}