#include "task.h"
#include "mmu.h"
#include "kmalloc.h"
#include "serial.h"
#include "list.h"
#include "mmu.h"
#include "elf.h"
#include "trap.h"

#define NO_TASKS 64 //Max 64 tasks for now
#define STACK_SIZE 4096

extern void set_kernel_stack(size_t);

list_t *task_list;

task_t *current = NULL;
list_t *ready_queue = NULL;
task_t *kernel_idle_task = NULL;

uint32_t last_process = 0;

#define PUSH(stack, type, value)                                               \
	stack -= sizeof(type);                                                 \
	*((type *)stack) = value

void exit_task()
{
	sys_exit(0);
}
void idle_task()
{
	while (1) {
		schedule();
	}
}
extern page_directory_t *current_directory;
task_t *spawn_init()
{
	task_t *init_task = create_task(NULL);

	init_task->state = STATE_RUNNING;

	//TODO, reuse kernel stack?
	init_task->stack = (vptr_t)kmalloc(STACK_SIZE);
	init_task->process->page_directory = clone_directory(current_directory);

	return init_task;
}
task_t *create_task(process_t *process)
{
	if (process == NULL) {
		process = kcalloc(sizeof(process_t));
		process->threads = list_create();
	}
	task_t *new_task = kcalloc(sizeof(task_t));
	new_task->process = process;
	list_append_item(process->threads, (vptr_t)new_task);

	return new_task;
}
task_t *spawn_idle()
{
	return copy_task((vptr_t)idle_task, NULL);
}
void tasking_install()
{
	task_list = list_create();
	ready_queue = list_create();
	current = spawn_init();
	kernel_idle_task = spawn_idle();

	//We are currently the init task
	switch_page_directory(current->process->page_directory);
	set_kernel_stack(current->stack + STACK_SIZE);
}

//Creates a new "thread" that runs the function fn
//Also allocates a new stack
//Does not start it yet
task_t *copy_task(vptr_t fn, vptr_t args)
{
	task_t *new_task = create_task(NULL);
	list_append_item(task_list, (vptr_t)new_task);

	new_task->stack = (vptr_t)kmalloc(STACK_SIZE);
	vptr_t stack = new_task->stack + STACK_SIZE;

	PUSH(stack, vptr_t, args);
	//Where fn returns to
	PUSH(stack, vptr_t, (vptr_t)exit_task);

	new_task->context.eip = fn;
	new_task->context.esp = stack;

	new_task->process->page_directory =
		clone_directory(current->process->page_directory);

	return new_task;
}
#include "fs.h"
extern void enter_userspace(vptr_t fn, vptr_t user_stack);
extern void interrupt_return();
void exec(file_t *file)
{
	free_user_mappings(current->process->page_directory);

	//TODO zero the BSS

	//Load ELF file
	Elf32_Ehdr header;

	vfs_read(file, &header, sizeof(header), 0);

	for (int x = 0; x < header.e_phnum; x++) {
		vptr_t ph_offset = x * header.e_phentsize + header.e_phoff;
		Elf32_Phdr ph;

		vfs_read(file, &ph, sizeof(ph), ph_offset);

		vptr_t section_end = PG_ROUND_UP(ph.p_vaddr + ph.p_memsz);
		size_t section_size = section_end - PG_ROUND_DOWN(ph.p_vaddr);

		mmap_flags_t flags = { //The elf file has overlapping sections
				       .IGNORE_PAGE_MAPPED = 1,
				       .MAP_IMMEDIATELY = 1
		};
		mmap(PG_ROUND_DOWN(ph.p_vaddr), section_size, flags);

		vfs_read(file, (uint8_t *)ph.p_vaddr, ph.p_filesz, ph.p_offset);

		//Set the rest of memory to zero
		vptr_t program_header_end = ph.p_vaddr + ph.p_filesz;
		memset((void *)program_header_end, 0, ph.p_memsz - ph.p_filesz);
	}

	//Setup user stack
	mmap_flags_t flags = { .MAP_IMMEDIATELY = 1 };
	mmap(USTACKTOP - STACK_SIZE, STACK_SIZE, flags);
	vptr_t stack = USTACKTOP;

	PUSH(stack, vptr_t, 0); //argv
	PUSH(stack, size_t, 0); //argc

	enter_userspace(header.e_entry, stack);
}
void make_task_ready(task_t *task)
{
	if (task == kernel_idle_task || task->state == STATE_FINISHED) {
		return;
	}

	task->state = STATE_READY;
	list_append_item(ready_queue, (vptr_t)task);
}

task_t *pick_next_task()
{
	if (ready_queue->len == 0) {
		return kernel_idle_task;
	}

	node_t *task_node = list_index(ready_queue, 0);
	task_t *task = (task_t *)task_node->value;

	list_remove(ready_queue, task_node);

	if (task->state != STATE_READY) {
		return pick_next_task();
	}

	return task;
}
extern void *_start;
uint32_t sys_clone(int_regs_t *regs)
{
	assert(current != NULL &&
	       "Current process does not exist, can't clone");

	vptr_t function_to_call = regs->ebx;
	vptr_t target_fn = regs->ecx;

	task_t *new_task = create_task(current->process);
	list_append_item(task_list, (vptr_t)new_task);

	new_task->stack = (vptr_t)kmalloc(STACK_SIZE);

	int_regs_t new_regs;
	memcpy(&new_regs, regs, sizeof(int_regs_t));
	//clone(fn) -> ebx has function address
	new_regs.eip = function_to_call;

	vptr_t kernel_stack = new_task->stack + STACK_SIZE;
	PUSH(kernel_stack, int_regs_t, new_regs);

	new_task->context.eip = (vptr_t)&interrupt_return;
	new_task->context.esp = kernel_stack;

	//Page directory remains the same
	new_task->process->page_directory = current->process->page_directory;

	mmap_flags_t flags = { .MAP_IMMEDIATELY = 1 };
	mmap(USTACKTOP2 - STACK_SIZE, STACK_SIZE, flags);

	vptr_t user_stack = USTACKTOP2;
	PUSH(user_stack, uint32_t, target_fn);
	new_regs.esp = user_stack;

	make_task_ready(new_task);

	return 0;
}

uint32_t sys_fork(int_regs_t *regs)
{
	assert(current != NULL && "Current process does not exist, can't fork");

	task_t *new_task = create_task(NULL);
	list_append_item(task_list, (vptr_t)new_task);

	new_task->stack = (vptr_t)kmalloc(STACK_SIZE);

	int_regs_t new_regs;
	memcpy(&new_regs, regs, sizeof(int_regs_t));
	new_regs.eax = 0;

	vptr_t stack = new_task->stack + STACK_SIZE;
	PUSH(stack, int_regs_t, new_regs);

	new_task->context.eip = (vptr_t)&interrupt_return;
	new_task->context.esp = stack;

	new_task->process->page_directory =
		clone_directory(current->process->page_directory);

	make_task_ready(new_task);

	return 999; //TODO return child PID
}
uint32_t sys_exit(int exitcode)
{
	debug_print("Exitcode 0x%x\n", exitcode);
	current->state = STATE_FINISHED;

	schedule();

	return 0; //Never happens but must do it because of macro setup
}
void free_process(process_t *process)
{
	free_page_directory(process->page_directory);
	assert(process->threads->len == 0);

	list_free(process->threads);
	kfree(process);
}
void free_task(task_t *task)
{
	process_t *process = task->process;
	//Cleanup
	list_remove_item(process->threads, (vptr_t)task);
	kfree((void *)task->stack);
	kfree(task);

	//Last thread in the process, we can clean the process
	if (process->threads->len == 0) {
		free_process(process);
	}
}
void schedule()
{
	task_t *next_task = pick_next_task();

	//Clean up old task
	while (next_task->state == STATE_FINISHED) {
		free_task(next_task);
		next_task = pick_next_task();
	}

	schedule_task(next_task);
}
void schedule_task(task_t *next_task)
{
	make_task_ready(current);
	if (next_task == current) {
		return;
	}

	context_t *old_context = (context_t *)&current->context;

	//Set up everything for the new process
	current = next_task;
	set_kernel_stack(current->stack + STACK_SIZE);

	switch_page_directory(current->process->page_directory);

	switch_context(old_context, &current->context);
}