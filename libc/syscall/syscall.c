#include "sys.h"
#include "syscalls.h"
#include "sys/types.h"
#include "coraxstd.h"

DEF_SYSCALL1(__NR_exit, _exit, int, exitcode)
DEF_SYSCALL0(__NR_fork, fork)
DEF_SYSCALL3(__NR_write, write, int, fd, char *, buf, int, size)

DEF_SYSCALL3(__NR_execve, execve, const char *, filename, char **, args,
	     char **, envs);
DEF_SYSCALL3(__NR_clone, clone2, void *, fn, void *, target_fn, void *,
	     child_stack);
DEF_SYSCALL1(__NR_sbrk, syscall_sbrk, uint32_t, size);
DEF_SYSCALL2(__NR_access, access, const char *, path, int, amode);

DEF_SYSCALL1(__NR_open, sys_open, const char *, path);
DEF_SYSCALL3(__NR_read, call_read, int, fd, void *, buf, size_t, n);
DEF_SYSCALL3(__NR_seek, call_seek, int, fd, long int, offset, int, whence);
DEF_SYSCALL3(__NR_update_screen, update_display, size_t, w, size_t, h, void *,
	     buffered_data);
DEF_SYSCALL2(__NR_time, gettimeofday, struct timeval *, p, void *, z);

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

int clone(void *fn, void *child_stack)
{
	return clone2(run_clone, fn, child_stack);
}