#include "proc.h"
#include "mmu.h"
#include "kmalloc.h"

#define NO_TASKS 64 //Max 64 tasks for now
#define STACK_SIZE 4096

extern void set_kernel_stack(size_t);

task_t tasks[NO_TASKS];
volatile task_t *current = tasks;

uint32_t last_process = 0;

/*
The following description is copied from the linux kernel, so I don't confuse the functionality
*/
/*
 * This creates a new process as a copy of the old one,
 * but does not actually start it yet.
 *
 * It copies the registers, and all the appropriate
 * parts of the process environment (as per the clone
 * flags). The actual kick-off is left to the caller.
 */
void clone(void (*func_addr)(void), void *new_stack)
{
	task_t *new_process = &tasks[last_process + 1];

	//Clear the old data
	memset((void *)new_process, 0, sizeof(task_t));

	new_process->context = new_stack - sizeof(context_t);
	new_process->context->eip = (size_t)func_addr;
	new_process->context->ebp = (size_t)new_stack;

	new_process->state = STATE_READY;

	last_process++;
}
void copy_task(task_t *task)
{
	task_t *new_process = &tasks[last_process + 1];

	//Copy the stack
	new_process->stack = (size_t)kmalloc(STACK_SIZE) + STACK_SIZE;
	memcpy((void *)new_process->stack - STACK_SIZE,
	       (void *)task->stack - STACK_SIZE, STACK_SIZE);

	//Update the esp and ebp
	//TODO WAIT, YOU HAVE TO UPDATE ALL THE LOCAL VARIABLES ON THE NEW STACK THIS WAY
	//new_process->ebp = new_process->esp =
}

task_t *pick_next_task()
{
	for (uint32_t i = 0; i <= last_process; i++) {
		if (tasks[i].state == STATE_READY && &tasks[i] != current) {
			return (task_t *)&tasks[i];
		}
	}

	return (task_t *)&tasks[0];
}
void schedule()
{
	task_t *next_task = pick_next_task();

	context_t **current_context = (context_t **)&current->context;
	current = next_task;

	set_kernel_stack(next_task->stack);

	cli(); //Next process is responsible to enable interrupts again
	switch_context(current_context, next_task->context);
	sti();
}
/*
	preempt_disable();

	//if (current->counter == 0) {
	for (uint32_t i = 0; i <= last_process; i++) {
		if (task[i].state == STATE_READY) {
			task[i].counter = task[i].priority;
			task[i].state = STATE_RUNNING;

			//Set the state as stopped for the currently running process
			current->state = STATE_READY;

			switch_to(&task[i]);

			break;
		}
	}
	//}
	preempt_enable();
}
void switch_to(struct task_struct *next)
{
	if (current == next) {
		return;
	}
	struct task_struct *prev = current;
	current = next;

	//Since ctx is first in current, this is fine
	switch_context((context_t **)&prev, (context_t *)next);
}*/