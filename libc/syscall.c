#include "sys.h"

DEF_SYSCALL1(0, exit, int, exitcode)
DEF_SYSCALL0(1, fork)
DEF_SYSCALL3(2, write, int, fd, char *, buf, int, size)
DEF_SYSCALL1(3, exec, char *, filename)
DEF_SYSCALL2(4, clone2, void *, fn, void *, target_fn);

void run_clone(void (*fn)())
{
	(*fn)();
	exit(0); //TODO add other exit codes
}

int clone(void *fn)
{
	return clone2(run_clone, fn);
}