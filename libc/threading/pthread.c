#include "pthread.h"
#include "malloc.h"
#include "syscalls.h"

#define PTHREAD_STACK_SIZE 0x100000

int pthread_create(pthread_t *thread, pthread_attr_t *attr,
		   void *(*start_routine)(void *), void *arg)
{
	void *stack = malloc(PTHREAD_STACK_SIZE);
	void *stack_top = stack + PTHREAD_STACK_SIZE;
	thread->stack = stack;
	thread->id = clone(start_routine, stack_top, arg);
	return 0;
}

int pthread_kill(pthread_t thread, int sig)
{
	kill(thread.id, sig);
}

int pthread_join(pthread_t thread, void **retval)
{
	if (retval == NULL) {
		waitpid(thread.id, NULL, 0);
	} else {
		waitpid(thread.id, *retval, 0);
	}
}
void pthread_exit(void *value)
{
	exit((int)value);
}