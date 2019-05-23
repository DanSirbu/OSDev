#include "include/sys.h"
#include "include/syscalls.h"
#include "types.h"

DEF_SYSCALL1(1, _exit, int, exitcode)
DEF_SYSCALL0(2, fork)
DEF_SYSCALL3(4, write, int, fd, char *, buf, int, size)

DEF_SYSCALL1(3, exec, char *, filename)
DEF_SYSCALL2(5, clone2, void *, fn, void *, target_fn);
DEF_SYSCALL1(6, syscall_sbrk, uint32_t, size);

void (*exit_funcs[10])(void);

int atexit(void (*function)(void))
{
	for (int x = 0; x < 9; x++) {
		if (exit_funcs[x] == NULL) {
			exit_funcs[x] = function;
			return 0;
		}
	}

	return -1;
}

void exit(uint32_t exitcode)
{
	for (int x = 9; x > 0; x--) {
		if (exit_funcs[x] != NULL) {
			exit_funcs[x]();
		}
	}
	_exit(exitcode);
}

void run_clone(void (*fn)())
{
	(*fn)();
	exit(0); //TODO add other exit codes
}

int clone(void *fn)
{
	return clone2(run_clone, fn);
}