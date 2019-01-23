#include "trap.h"
#include "task.h"
#include "syscalls.h"

int sys_write(int fd, char *buf, int size)
{
	debug_print(buf);
	return 1;
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
	default:
		debug_print("Unhandled syscall\n");
	}
}
void syscalls_install()
{
	register_isr_handler(SYSCALL_NO, syscall);
}