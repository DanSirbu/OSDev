#include "trap.h"
#include "task.h"

int write(int_regs_t *regs)
{
	debug_print((char *)regs->ecx);
	return 1;
}

void syscall(int_regs_t *regs)
{
	debug_print("Syscall 0x%x\n", regs->eax);
	switch (regs->eax) {
	case 0:
		task_exit((int)regs->ebx);
		break;
	case 1:
		regs->eax = fork(regs);
		break;
	case 2:
		regs->eax = write(regs);
		break;
	default:
		debug_print("Unhandled syscall\n");
	}
}
void syscalls_install()
{
	register_isr_handler(SYSCALL_NO, syscall);
}