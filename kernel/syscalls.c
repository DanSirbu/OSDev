#include "trap.h"

void syscall(int_regs_t *regs)
{
	debug_print("Syscall %x", regs->eax);
}
void syscalls_install()
{
	register_isr_handler(SYSCALL_NO, syscall);
}