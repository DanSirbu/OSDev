#include "spinlock.h"
#include "circularqueue.h"
#include "kmalloc.h"
#include "list.h"
#include "assert.h"

/** Checks whether the circular queue is empty or not. */
//Only visible to the circularqueue.c file to make it thread safe
static bool CircularQueueIsEmpty(CircularQueue *obj)
{
	return obj->front == -1 || obj->rear == -1;
}

/** Checks whether the circular queue is full or not. */
//Only visible to the circularqueue.c file to make it thread safe
static bool CircularQueueIsFull(CircularQueue *obj)
{
	return ((obj->rear + 1) % (int)obj->size) == obj->front;
}

/** Insert an element into the circular queue. Return true if the operation is successful. */
static bool CircularQueueEnQueueUnsafe(CircularQueue *obj, int value)
{
	if (CircularQueueIsFull(obj)) {
		return false;
	}
	if (CircularQueueIsEmpty(obj)) {
		obj->front = obj->rear = 0;
	} else {
		obj->rear = (obj->rear + 1) % (int)obj->size;
	}
	obj->data[obj->rear] = value;
	return true;
}

/** Delete an element from the circular queue. Return true if the operation is successful. */
static bool CircularQueueDeQueueUnsafe(CircularQueue *obj)
{
	if (CircularQueueIsEmpty(obj)) {
		return false;
	}
	if (obj->front == obj->rear) {
		obj->front = -1;
	} else {
		obj->front = (obj->front + 1) % obj->size;
	}
	return true;
}

/** Get the front item from the queue. */
static int CircularQueueFrontUnsafe(CircularQueue *obj)
{
	if (CircularQueueIsEmpty(obj)) {
		return NULL;
	}
	return obj->data[obj->front];
}

/** Get the last item from the queue. */
/*static int CircularQueueRearUnsafe(CircularQueue *obj)
{
	if (CircularQueueIsEmpty(obj)) {
		return -1;
	}
	return obj->data[obj->rear];
}*/

CircularQueue *CircularQueueCreate(int k)
{
	CircularQueue *queue = kmalloc(sizeof(CircularQueue));
	queue->data = kmalloc(k * sizeof(uint32_t));
	queue->size = k;

	queue->front = -1;
	queue->rear = -1;

	queue->front_lock = 0;
	queue->rear_lock = 0;

	//TODO, move this out of queue?
	queue->write_queue = list_safe_create();
	queue->read_queue = list_safe_create();
	return queue;
}

void CircularQueueFree(CircularQueue *obj)
{
	assert(obj->write_queue->len == 0);
	assert(obj->read_queue->len == 0);
	list_safe_free(obj->write_queue);
	list_safe_free(obj->read_queue);
	kfree(obj->data);
	kfree(obj);
}

int CircularQueueFront(CircularQueue *obj)
{
	spinlock_acquire(&obj->front_lock);
	int ret = CircularQueueFrontUnsafe(obj);
	spinlock_release(&obj->front_lock);
	return ret;
}
bool CircularQueueEnQueue(CircularQueue *obj, int value)
{
	spinlock_acquire(&obj->rear_lock);
	bool ret = CircularQueueEnQueueUnsafe(obj, value);
	spinlock_release(&obj->rear_lock);

	return ret;
}
bool CircularQueueDeQueue(CircularQueue *obj)
{
	spinlock_acquire(&obj->front_lock);
	bool ret = CircularQueueDeQueueUnsafe(obj);
	spinlock_release(&obj->front_lock);

	return ret;
}