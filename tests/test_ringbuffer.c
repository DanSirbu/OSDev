#include "ringbuffer.h"
#include "assert.h"

void test_readamount()
{
	ring_buffer_t *ring_buffer = create_ring_buffer(5);
	ring_buffer->read_ptr = 3;
	ring_buffer->write_ptr = 2;
	assert(get_read_amount(ring_buffer) == 4);

	ring_buffer->read_ptr = 2;
	ring_buffer->write_ptr = 3;
	assert(get_read_amount(ring_buffer) == 1);

	ring_buffer->read_ptr = 0;
	ring_buffer->write_ptr = 0;
	assert(get_read_amount(ring_buffer) == 0);

	free_ring_buffer(ring_buffer);
}

void test_ringbuffer()
{
	test_readamount();
}