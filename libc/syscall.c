#include "include/sys.h"
#include "include/syscalls.h"
#include "types.h"
#include "coraxstd.h"

DEF_SYSCALL1(__NR_exit, _exit, int, exitcode)
DEF_SYSCALL0(__NR_fork, fork)
DEF_SYSCALL3(__NR_write, write, int, fd, char *, buf, int, size)

DEF_SYSCALL1(__NR_execve, exec, char *, filename)
DEF_SYSCALL2(__NR_clone, clone2, void *, fn, void *, target_fn);
DEF_SYSCALL1(__NR_sbrk, syscall_sbrk, uint32_t, size);
DEF_SYSCALL2(__NR_access, access, const char *, path, int, amode);

DEF_SYSCALL1(__NR_open, sys_open, const char *, path);
DEF_SYSCALL3(__NR_read, call_read, int, fd, void *, buf, size_t, n);
DEF_SYSCALL3(__NR_seek, call_seek, int, fd, size_t, offset, int, whence);

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