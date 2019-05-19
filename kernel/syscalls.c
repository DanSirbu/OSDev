#include "trap.h"
#include "task.h"
#include "syscalls.h"
#include "fs.h"
#include "vfs.h"
#include "display.h"

int sys_write(int fd, char *buf, int size)
{
	debug_print("sys_write: %d %s %d\n", fd, buf, size);
	return 1;
}
int sys_exec(char *filename)
{
	file_t *file = vfs_open(filename);
	if (file == NULL) {
		return -1;
	}
	exec(file);

	//Does not get here
	return 0;
}
extern task_t *current;
vptr_t sys_sbrk(uint32_t size)
{
	size = PG_ROUND_UP(size);
	vptr_t old_heap_addr = current->process->heap;

	mmap_flags_t flags = { .MAP_IMMEDIATELY = 1,
			       .IGNORE_PAGE_MAPPED = 0,
			       .IGNORE_FRAME_REUSE = 0 };
	mmap(old_heap_addr, size, flags);
	current->process->heap = old_heap_addr + size;
	return old_heap_addr;
}
vptr_t sys_update_display(uint32_t w, uint32_t h, uint32_t *buffered_data)
{
	int z = w + h;
	display_update(buffered_data);
	z++;
	return 0;
}
void syscall(int_regs_t *regs)
{
	print(LOG_INFO, "Syscall 0x%x\n", regs->eax);
	switch (regs->eax) {
		DEF_SYSCALL1(0, exit, int, exitcode)
	case 1:
		regs->eax = sys_fork(regs); //Special case
		break;
		DEF_SYSCALL3(2, write, int, fd, char *, buf, int, size)
		DEF_SYSCALL1(3, exec, char *, filename);
	case 4:
		sys_clone(regs);
		break;
		DEF_SYSCALL1(5, sbrk, uint32_t, size);
		DEF_SYSCALL3(10, update_display, size_t, w, size_t, h,
			     uint32_t *, buffered_data);
	default:
		print(LOG_ERROR, "Unhandled syscall 0x%x\n", regs->eax);
	}
}
void syscalls_install()
{
	register_isr_handler(SYSCALL_NO, syscall);
}