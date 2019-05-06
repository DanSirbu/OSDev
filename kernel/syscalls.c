#include "trap.h"
#include "task.h"
#include "syscalls.h"
#include "fs.h"
#include "vfs.h"

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
void syscall(int_regs_t *regs)
{
	debug_print("Syscall 0x%x\n", regs->eax);
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
	default:
		debug_print("Unhandled syscall\n");
	}
}
void syscalls_install()
{
	register_isr_handler(SYSCALL_NO, syscall);
}