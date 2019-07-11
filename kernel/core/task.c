#include "task.h"
#include "mmu.h"
#include "kmalloc.h"
#include "serial.h"
#include "list.h"
#include "mmu.h"
#include "elf.h"
#include "trap.h"
#include "vfs.h"
#include "debug.h"
#include "spinlock.h"

/* Prototypes */
void handle_signals();

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

pid_t NEXT_PID = 0;
int NEXT_PID_LOCK = 0;

pid_t getNextPID()
{
	int ret;
	spinlock_acquire(&NEXT_PID_LOCK);
	ret = NEXT_PID++;
	spinlock_release(&NEXT_PID_LOCK);
	return ret;
}
void exit_task()
{
	sys_exit(0);
}
void idle_task()
{
	while (1) {
		sti();
		schedule();
	}
}
extern page_directory_t *current_directory;
task_t *spawn_init()
{
	task_t *init_task = create_task(NULL, "init");

	init_task->state = STATE_INIT;

	//TODO, reuse kernel stack?
	init_task->stack = (size_t)kmalloc(STACK_SIZE);
	init_task->process->page_directory = clone_directory(current_directory);

	list_enqueue(task_list, init_task);

	return init_task;
}
task_t *create_task(process_t *process, const char *name)
{
	if (process == NULL) { //First task
		process = kcalloc(sizeof(process_t));
		process->threads = list_create();
		process->signals = list_create();
	}
	task_t *new_task = kcalloc(sizeof(task_t));
	new_task->process = process;
	new_task->id = getNextPID();
	new_task->exit_waiting_threads = list_safe_create();
	new_task->name = strdup(name);
	list_enqueue(process->threads, new_task);

	return new_task;
}
task_t *spawn_idle()
{
	task_t *task = copy_task((size_t)idle_task, (size_t)NULL);
	set_task_name(task, "idle");
	return task;
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
task_t *copy_task(size_t fn, size_t args)
{
	task_t *new_task = create_task(NULL, "kthread");
	list_enqueue(task_list, new_task);

	new_task->stack = (size_t)kmalloc(STACK_SIZE);
	size_t stack = new_task->stack + STACK_SIZE;

	PUSH(stack, size_t, args);
	//Where fn returns to
	PUSH(stack, size_t, (size_t)exit_task);

	new_task->context.eip = fn;
	new_task->context.esp = stack;

	new_task->process->page_directory =
		clone_directory(current->process->page_directory);

	return new_task;
}

extern void enter_userspace(size_t fn, size_t user_stack);
extern void interrupt_return();

size_t push_array_to_stack(size_t stacktop, char *arr[])
{
	size_t stack = stacktop;
	uint32_t num_items = array_length(arr);

	assert(num_items > 0);

	char *envs_ptr[num_items];
	//Copy strings into the stack
	for (size_t x = 0; x < num_items; x++) {
		uint32_t item_size =
			strlen(arr[x]) + 1; //Include null terminator
		stack -= item_size;
		memcpy((void *)stack, arr[x], item_size);
		envs_ptr[x] = (char *)stack;
	}

	//Push string pointers in reverse order
	PUSH(stack, void *, NULL); //Don't forget terminating null pointer
	for (int x = num_items - 1; x >= 0; x--) {
		PUSH(stack, char *, envs_ptr[x]);
	}
	PUSH(stack, char **, (char **)(stack + sizeof(char **)));

	return stack;
}

int execve(char *filename, char *argv[], char *envp[])
{
	file_t *file = vfs_open(filename);
	if (file == NULL) {
		return -1;
	}
	print(LOG_INFO, "Running %s\n", filename);

	//Load ELF file
	Elf32_Ehdr header;
	vfs_read(file, &header, 0, sizeof(header));
	if (!(header.e_ident[EI_MAG0] == ELFMAG0 &&
	      header.e_ident[EI_MAG1] == ELFMAG1 &&
	      header.e_ident[EI_MAG2] == ELFMAG2 &&
	      header.e_ident[EI_MAG3] == ELFMAG3)) {
		print(LOG_WARNING, "execve: not an elf file %s\n", filename);
		return -1;
	}

	//Clean up userspace
	free_user_mappings(current->process->page_directory);
	//TODO zero the BSS

	size_t heap_start = 0;
	for (int x = 0; x < header.e_phnum; x++) {
		size_t ph_offset = x * header.e_phentsize + header.e_phoff;

		Elf32_Phdr ph;
		vfs_read(file, &ph, ph_offset, sizeof(ph));

		size_t segment_end = PG_ROUND_UP(ph.p_vaddr + ph.p_memsz);
		size_t segment_size = segment_end - PG_ROUND_DOWN(ph.p_vaddr);

		mmap_flags_t flags = { //The elf file has overlapping sections
				       .IGNORE_PAGE_MAPPED = 1,
				       .MAP_IMMEDIATELY = 1
		};
		mmap(PG_ROUND_DOWN(ph.p_vaddr), segment_size, flags);

		vfs_read(file, (uint8_t *)ph.p_vaddr, ph.p_offset, ph.p_filesz);

		//filesz = segment size in file
		//memsz = segment size in memory, can be > filesz
		//Set the rest of memory to zero
		size_t program_header_end = ph.p_vaddr + ph.p_filesz;
		memset((void *)program_header_end, 0, ph.p_memsz - ph.p_filesz);

		if (segment_end > heap_start) {
			heap_start = segment_end;
		}
	}
	assert(heap_start != 0);
	heap_start = PG_ROUND_UP(heap_start + PGSIZE -
				 1); //Add one guard page between code and heap

	current->process->heap = heap_start;

	//Setup user stack
	mmap_flags_t flags = { .MAP_IMMEDIATELY = 1 };
#undef STACK_SIZE
#define STACK_SIZE (0x1000 * 500)
	mmap(USTACKTOP - STACK_SIZE, STACK_SIZE, flags);
	size_t stack = USTACKTOP;
#undef STACK_SIZE
#define STACK_SIZE 0x1000

	stack = push_array_to_stack(stack, envp);
	size_t envp_pointer_value = *(size_t *)stack;
	stack += sizeof(char **);

	stack = push_array_to_stack(stack, argv);
	size_t argv_pointer_value = *(size_t *)stack;
	stack += sizeof(char **);

	//main(argc, argv, envp)
	PUSH(stack, char **, (char **)envp_pointer_value);
	PUSH(stack, char **, (char **)argv_pointer_value);
	PUSH(stack, int, array_length(argv));

	//Set new process name
	set_task_name(current, getFilenameNoExt(filename));

	//Cleanup
	vfs_close(file);
	kfree(filename);
	kfree_arr(argv);
	kfree_arr(envp);

	//Start user process
	enter_userspace(header.e_entry, stack);

	//Should not get here
	return -1;
}
void make_task_ready(task_t *task)
{
	if (task == kernel_idle_task || task->state == STATE_FINISHED) {
		return;
	}

	task->state = STATE_READY;
	list_enqueue(ready_queue, task);
}

task_t *pick_next_task()
{
	if (ready_queue->len == 0) {
		return kernel_idle_task;
	}

	task_t *task = list_dequeue(ready_queue);

	/*if (task->state != STATE_READY) {
		return pick_next_task();
	}*/

	return task;
}
extern void *_start;
uint32_t sys_clone(void *fn, void *child_stack, void *arg)
{
	assert(current != NULL &&
	       "Current process does not exist, can't clone");
	assert(current->process->userspace_variables.clone_func_caller != NULL);

	task_t *new_task = create_task(current->process, current->name);
	list_enqueue(task_list, new_task);

	//Page directory remains the same
	new_task->process->page_directory = current->process->page_directory;

	//Parameters for the clone userspace handler
	size_t user_stack_top = (size_t)child_stack;
	PUSH(user_stack_top, void *, arg);
	PUSH(user_stack_top, void *, fn);
	PUSH(user_stack_top, size_t, 0x99999); //Bogus function return address

	//Kernel stack
	new_task->stack = (size_t)kmalloc(STACK_SIZE);
	size_t kernel_stack_top = new_task->stack + STACK_SIZE;
	//enter_userspace(func, userstack) parameters
	PUSH(kernel_stack_top, void *, (void *)user_stack_top); //userstack
	PUSH(kernel_stack_top, void *,
	     current->process->userspace_variables.clone_func_caller); //func
	PUSH(kernel_stack_top, size_t, 0x99999); //Bogus function return address

	//Kernel "saved" context
	new_task->context.esp = kernel_stack_top;
	new_task->context.eip = (size_t)&enter_userspace;

	make_task_ready(new_task);

	return new_task->id;
}

uint32_t sys_fork(int_regs_t *regs)
{
	assert(current != NULL && "Current process does not exist, can't fork");

	task_t *new_task = create_task(NULL, current->name);
	//copy fds
	memcpy(new_task->process->files, current->process->files,
	       sizeof(new_task->process->files));

	list_enqueue(task_list, new_task);

	new_task->stack = (size_t)kmalloc(STACK_SIZE);

	int_regs_t userspace_regs;
	memcpy(&userspace_regs, regs, sizeof(userspace_regs));
	userspace_regs.eax = 0; //Child pid returned value

	size_t stack = new_task->stack + STACK_SIZE;
	PUSH(stack, int_regs_t, userspace_regs);

	new_task->context.eip = (size_t)&interrupt_return;
	new_task->context.esp = stack;

	new_task->process->page_directory =
		clone_directory(current->process->page_directory);

	make_task_ready(new_task);

	return new_task->id;
}
uint32_t sys_exit(int exitcode)
{
	current->state = STATE_FINISHED;
	current->exitcode = exitcode;
	wakeup_queue(current->exit_waiting_threads);

	schedule();

	print(LOG_ERROR, "Exited process (pid: %d) continued\n", current->id);
	assert(1 == 2); //Should never get here
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
	assert(task->state == STATE_FINISHED);

	process_t *process = task->process;
	list_remove_item(process->threads, task);

	//Free task
	assert(task->exit_waiting_threads->len == 0);
	list_safe_free(task->exit_waiting_threads);
	kfree((void *)task->stack);
	kfree(task->name);
	kfree(task);

	//Last thread in the process, we can clean the process
	if (process->threads->len == 0) {
		free_process(process);
	}
}
//Timedelta is milliseconds
void update_timer(uint32_t timeDelta)
{
	timeDelta *= 1000; //Change timedelta from millisecond to microseconds
	struct itimerval *procTimer = &current->process->timer;

	if (procTimer->it_interval.tv_sec == 0 &&
	    procTimer->it_interval.tv_usec == 0) {
		return;
	}

	procTimer->it_value.tv_usec += timeDelta;
	if (procTimer->it_value.tv_usec > 1000000) {
		uint32_t additionalSeconds =
			procTimer->it_value.tv_usec / 1000000;
		procTimer->it_value.tv_sec += additionalSeconds;
		procTimer->it_value.tv_usec -= additionalSeconds * 1000000;
	}

	if (procTimer->it_value.tv_usec >= procTimer->it_interval.tv_usec &&
	    procTimer->it_value.tv_sec >= procTimer->it_interval.tv_sec) {
		//memset(procTimer, 0, sizeof(struct itimerval));
		procTimer->it_value = procTimer->it_interval;
		assert(sys_kill(sys_getPID(), SIGALRM) == 0);
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
	if (current->state != STATE_FINISHED &&
	    current->state != STATE_SLEEPING) {
		make_task_ready(current);
	} else if (current->state == STATE_FINISHED) {
		list_enqueue(ready_queue, current); //Enqueue for cleanup
	}

	//If the task is the same, we don't need to load it again
	if (next_task == current) {
		handle_signals();
		return;
	}

	context_t *old_context = (context_t *)&current->context;

	//Set up everything for the new process
	current = next_task;
	set_kernel_stack(current->stack + STACK_SIZE);

	switch_page_directory(current->process->page_directory);

	handle_signals();
	switch_context(old_context, &current->context);
}
void handle_signals()
{
	//Handle signals
	if (current->process->signals->len > 0) {
		size_t userSigHandler =
			(size_t)current->process->userspace_variables
				.SignalHandler;
		assert(userSigHandler != 0);
		assert(current->int_regs != NULL);

		int sig = (int)list_dequeue(current->process->signals);
		//Note: if this is changed, libc/main/signal.S must be changed as well
		//We push the eip to the stack
		//Then the signal
		//Then change the eip to the signal handler which is responsible for preserving the registers and jumping to eip when finished

		size_t stack = current->int_regs->useresp;
		PUSH(stack, size_t, current->int_regs->eip);
		PUSH(stack, int, sig);
		current->int_regs->useresp = stack;

		current->int_regs->eip = userSigHandler;
	}
}
void set_int_regs(int_regs_t *regs)
{
	current->int_regs = regs;
}

int getNextFD(process_t *proc)
{
	for (int i = 0; i < MAX_FD; i++) {
		if (proc->files[i] == NULL) {
			return i;
		}
	}
	return -1;
}

void wakeup_queue(threaded_list_t *queue)
{
	task_t *task = list_safe_dequeue(queue);
	while (task != NULL) {
		make_task_ready(task);
		task = list_safe_dequeue(queue);
	}
}
void sleep_on(threaded_list_t *queue)
{
	assert(current != kernel_idle_task);
	current->state = STATE_SLEEPING;
	list_safe_enqueue(queue, current);
	schedule();
}

int sys_kill(pid_t pid, int sig)
{
	//TODO if pid == 0, send to process group
	//TODO if error, return -1
	if (pid == 0) {
		return -1;
	}
	task_t *task = getTask(pid);
	if (task == NULL) {
		return -1;
	}
	list_enqueue(task->process->signals, (void *)sig);

	schedule();

	return 0;
}

pid_t sys_getPID()
{
	return current->id;
}
task_t *getTask(pid_t pid)
{
	foreach_list(task_list, curTask)
	{
		task_t *task = (void *)curTask->value;
		if (task->id == pid) {
			return task;
		}
	}

	return NULL;
}
int getNumTasksInTasklist()
{
	int numTasks = 0;
	foreach_list(task_list, curTask)
	{
		numTasks++;
	}
	return numTasks;
}
void set_task_name(task_t *task, const char *name)
{
	free(task->name);
	task->name = strdup(name);
}
pid_t sys_waitpid(pid_t pid, int *stat_loc, int options)
{
	assert(options == 0 && pid > 0); //TODO, make sys_waitpid handle options

	task_t *task = getTask(pid);
	if (task == NULL) {
		return -1;
	}

	//Wait for task to finish
	if (task->state != STATE_FINISHED) {
		list_safe_enqueue(task->exit_waiting_threads, current);
		current->state = STATE_SLEEPING;
		schedule();
	}

	if (stat_loc != NULL) {
		*stat_loc = task->exitcode;
	}

	return task->id;
}