#pragma once
#include "types.h"

typedef struct {
	uint32_t *data;
	uint32_t size;
	int front; //First item
	int rear; //last item
	int front_lock;
	int rear_lock;
} CircularQueue;

CircularQueue *CircularQueueCreate(int k);
int CircularQueueFront(CircularQueue *obj);
bool CircularQueueEnQueue(CircularQueue *obj, int value);
bool CircularQueueDeQueue(CircularQueue *obj);
void CircularQueueFree(CircularQueue *obj);