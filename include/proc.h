#pragma once
#include "types.h"

typedef struct context {
	size_t edi;
	size_t esi;
	size_t ebx;
	size_t ebp;
	size_t eip;
} context_t;

struct cpu {
	context_t *scheduler_ctx;
};

enum STATES { STATE_READY = 1, STATE_RUNNING = 2, STATE_FINISHED = 0 };

//https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson04/rpi-os.md
//The counter allows preemption
//We decrease it by one at every tick and schedule another task when it reaches 0
//Priority = what we set the counter to when we schedule the process for a time slice
//If criticalRegion is non-zero, then it means process should not be interrupted

/*So here is the thing, why does everybody say the kernel has processes and threads?
THOSE DON'T EXIST AT ALL!
All the kernel (specifically, the scheduler) knows about is tasks

clone(task) = copy registers and create new stack, page_directory stays the same = analogous to thread
fork(task) = copy everything = analogous to process
*/

//Oh and idle task exists because there is no task left to run
typedef struct {
	context_t *context;

	enum STATES state;
	uint32_t counter;
	uint32_t priority;

	page_directory_t page_directory;
} task_t;

/*
Note: when we switch a context, we basically do a thread switch
Then, if we also load a new process
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.

//Note: context is stored at the top of the stack (bottom address really since the stack grows down in x86), the register values are poped
// and we still have the function calls and parameters to play with
//I.e. 
//
////Process A////
//
//doSomething(varA) {
//context_switch(otherProcess)
//NEXT INSTRUCTION
//}

//Now other process calls context_switch back to process A

//The stack of process A looks like this before the switch:
//varA
context_struct {
//eip of NEXT INSTRUCTION
//			<- Pushed by switch_context call in process A
ebp
ebx
esi
edi

*/
void switch_context(context_t **cur_context, context_t *new_context);
void schedule();
void copy_process(void (*func_addr)(void));
void clone(void (*func_addr)(void), void *new_stack);