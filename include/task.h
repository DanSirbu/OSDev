#pragma once
#include "sys/types.h"
#include "mmu.h"
#include "trap.h"
#include "fs.h"
#include "list.h"
#include "coraxstd.h"

typedef struct context {
	size_t eip;
	size_t esp;

	size_t ebp;
	size_t ebx;
	size_t esi;
	size_t edi;
} __attribute__((packed)) context_t;

struct cpu {
	context_t *scheduler_ctx;
};

enum STATES {
	STATE_INIT = 0,
	STATE_READY = 1,
	STATE_SLEEPING = 2,
	STATE_FINISHED = 3
};

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
#define MAX_FD 10
#define MAX_FD_INDEX (MAX_FD - 1)

typedef struct {
	pid_t pid;

	page_directory_t *page_directory;

	size_t heap;

	list_t *threads;

	file_t *files[MAX_FD];
	list_t *signals;

	struct userspace_vars userspace_variables;

	struct itimerval timer;
} process_t;

typedef struct {
	process_t *process;

	int_regs_t *int_regs;

	context_t context; //Kernel context
	size_t stack; //Kernel stack

	enum STATES state;
} task_t; //task = thread

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
void tasking_install();
void switch_context(context_t *cur_context, context_t *new_context);

pid_t getNextPID();
uint32_t sys_exit(int exitcode);
uint32_t sys_fork(int_regs_t *regs);
void schedule();
void make_task_ready(task_t *task);
task_t *create_task(process_t *process);
void schedule_task(task_t *next_task);
void set_int_regs(int_regs_t *regs);

task_t *copy_task(size_t fn, size_t args);
void clone(void (*func_addr)(void), void *new_stack);
int execve(const char *filename, char *argv[], char *envp[]);
uint32_t sys_clone(void *fn, void *target_fn, void *child_stack);

void wakeup_queue(threaded_list_t *queue);
void sleep_on(threaded_list_t *queue);

int getNextFD(process_t *proc);