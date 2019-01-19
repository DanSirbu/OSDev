#include "trap.h"
#include "task.h"

void syscall(int_regs_t *regs)
{
	debug_print("Syscall 0x%x\n", regs->eax);
	switch (regs->eax) {
	case 0:
		task_exit(regs->ebx);
		break;
	case 1:
		regs->eax = fork();
		break;
	default:
		debug_print("Unhandled syscall\n");
	}
}
void syscalls_install()
{
	register_isr_handler(SYSCALL_NO, syscall);
}