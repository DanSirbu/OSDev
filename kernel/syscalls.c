#include "trap.h"
#include "task.h"

void syscall(int_regs_t *regs)
{
	debug_print("Syscall 0x%x\n", regs->eax);
	if(regs->eax == 0) {
		task_exit(regs->ebx);
	}
}
void syscalls_install()
{
	register_isr_handler(SYSCALL_NO, syscall);
}