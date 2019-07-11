#pragma once

#include "coraxstd.h"
#include "syscalls.h"

typedef struct {
	void *stack;
	int id;
} pthread_t;

typedef unsigned int pthread_attr_t;

extern int pthread_create(pthread_t *thread, pthread_attr_t *attr,
			  void *(*start_routine)(void *), void *arg);

extern int pthread_join(pthread_t thread, void **retval);

extern void pthread_exit(void *value);
extern int pthread_kill(pthread_t thread, int sig);