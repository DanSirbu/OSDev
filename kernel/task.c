#include "task.h"
#include "mmu.h"
#include "kmalloc.h"
#include "serial.h"

#define NO_TASKS 64 //Max 64 tasks for now
#define STACK_SIZE 4096

extern void set_kernel_stack(size_t);

task_t task_list[NO_TASKS];
volatile task_t *current = task_list;

uint32_t last_process = 0;

#define PUSH(stack, type, value)                                               \
	stack -= sizeof(type);                                                 \
	*((type *)stack) = value

void exit_task()
{
	kpanic_fmt("TASK RETURN");
	while (1)
		;
}
//Creates a new "thread" that runs the function fn
//Also allocates a new stack
//Does not start it yet
task_t *copy_task(vptr_t fn, vptr_t args)
{
	task_t *new_task = &task_list[++last_process];

	//Clear the old data
	memset((void *)new_task, 0, sizeof(task_t));

	vptr_t stack = (vptr_t)(kmalloc(STACK_SIZE) + STACK_SIZE);
	new_task->stack = stack;

	PUSH(stack, vptr_t, args);
	//Where fn returns to
	PUSH(stack, vptr_t, (vptr_t)exit_task);

	new_task->context.eip = fn;
	new_task->context.esp = stack;

	new_task->page_directory = current->page_directory;

	return new_task;
}
extern void enter_userspace(vptr_t fn, vptr_t stack);
void exec(task_t *task, vptr_t fn)
{
	vptr_t stack = (vptr_t)kmalloc(0x1000);
	enter_userspace(stack, fn);
}

task_t *pick_next_task()
{
	for (uint32_t i = 0; i <= last_process; i++) {
		if (task_list[i].state == STATE_READY &&
		    &task_list[i] != current) {
			return (task_t *)&task_list[i];
		}
	}

	return (task_t *)current;
}
void schedule()
{
	task_t *next_task = pick_next_task();
	if (next_task == current) {
		return;
	}

	context_t *current_context = (context_t *)&current->context;
	current = next_task;

	set_kernel_stack(next_task->stack);

	switch_context(current_context, &next_task->context);
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