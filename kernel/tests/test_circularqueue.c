#include "circularqueue.h"
#include "assert.h"

void test_circularqueue()
{
	debug_print(" Testing CircularQueue ");
	CircularQueue *queue = CircularQueueCreate(3);
	assert(CircularQueueDeQueue(queue) == false);

	/* Test 1 item */
	assert(CircularQueueEnQueue(queue, 1) == true);
	assert(CircularQueueFront(queue) == 1);
	assert(CircularQueueBack(queue) == 1);
	assert(CircularQueueDeQueue(queue) == true);
	assert(CircularQueueDeQueue(queue) == false);
	assert(CircularQueueFront(queue) == (int)NULL);
	assert(CircularQueueBack(queue) == (int)NULL);

	assert(CircularQueueEnQueue(queue, 1) == true);
	assert(CircularQueueEnQueue(queue, 2) == true);
	assert(CircularQueueEnQueue(queue, 3) == true);
	assert(CircularQueueEnQueue(queue, 4) == false);
	assert(CircularQueueDeQueue(queue) == true); //2, 3 left

	assert(CircularQueueFront(queue) == 2);
	assert(CircularQueueBack(queue) == 3);
	assert(CircularQueueEnQueue(queue, 4) == true); //2, 3, 4 left
	assert(CircularQueueEnQueue(queue, 5) == false);

	assert(CircularQueueDeQueue(queue) == true); //3, 4 left
	assert(CircularQueueFront(queue) == 3);
	assert(CircularQueueDeQueue(queue) == true); //4 left
	assert(CircularQueueFront(queue) == 4);
	assert(CircularQueueDeQueue(queue) == true);
	assert(CircularQueueFront(queue) == (int)NULL);
	assert(CircularQueueDeQueue(queue) == false);

	CircularQueueFree(queue);
}