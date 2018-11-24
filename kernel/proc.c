#include "proc.h"
#include "mmu.h"
#include "kmalloc.h"

#define NO_TASKS 64 //Max 64 tasks for now

process_t task[NO_TASKS];
volatile process_t *current = task;

uint32_t last_process = 0;

//Note: init task is the first one
//The scheduler runs on the currently running process

//It checks to see if a new process should be run
#define STATE_READY 1
#define STATE_RUNNING 2
#define STATE_TERMINATED 3

void preempt_disable()
{
}
void preempt_enable()
{
}
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
	process_t *new_process = &task[last_process + 1];

	//Clear the old data
	memset((void *)new_process, 0, sizeof(process_t));

	new_process->context = new_stack - sizeof(context_t);
	new_process->context->eip = (size_t)func_addr;
	new_process->context->ebp = (size_t)new_stack;

	new_process->state = STATE_READY;

	last_process++;
}
void spawn_init()
{
	process_t proc;
	//proc->context.eip = 10;
}

void schedule()
{
	for (uint32_t i = 0; i <= last_process; i++) {
		if (task[i].state == STATE_READY && &task[i] != current) {
			context_t **current_context =
				(context_t **)&current->context;
			current = &task[i];
			asm volatile("cli");
			switch_context(current_context, task[i].context);
			asm volatile("sti");
			break;
		}
	}
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