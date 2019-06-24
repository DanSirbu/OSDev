#pragma once
#include "sys/types.h"
#include "list.h"

typedef struct {
	uint32_t *data;
	uint32_t size;
	int front; //First item
	int rear; //last item
	int front_lock;
	int rear_lock;

	threaded_list_t *write_queue;
	threaded_list_t *read_queue;
} CircularQueue;

CircularQueue *CircularQueueCreate(int k);
int CircularQueueFront(CircularQueue *obj);
bool CircularQueueEnQueue(CircularQueue *obj, int value);
bool CircularQueueDeQueue(CircularQueue *obj);
void CircularQueueFree(CircularQueue *obj);