#pragma once
#include "sys/types.h"
#include "coraxstd.h"

#define DECL_SYSCALL0(unused, name) int name();
#define DECL_SYSCALL1(unused, name, P1, p1) int name(P1 p1);
#define DECL_SYSCALL2(unused, name, P1, p1, P2, p2) int name(P1 p1, P2 p2);
#define DECL_SYSCALL3(unused, name, P1, p1, P2, p2, P3, p3)                    \
	int name(P1 p1, P2 p2, P3 p3);

DECL_SYSCALL1(NULL, _exit, int, exitcode)
DECL_SYSCALL0(NULL, fork)
DECL_SYSCALL3(NULL, write, int, fd, char *, buf, int, size)
DECL_SYSCALL3(__NR_clone, clone2, void *, fn, void *, target_fn, void *,
	      child_stack);
DECL_SYSCALL1(NULL, syscall_sbrk, uint32_t, size);
DECL_SYSCALL2(__NR_access, access, const char *, path, int, amode);

DECL_SYSCALL1(__NR_open, sys_open, const char *, path);
DECL_SYSCALL3(__NR_read, call_read, int, fd, void *, buf, size_t, n);
DECL_SYSCALL3(__NR_seek, call_seek, int, fd, long int, offset, int, whence);

DECL_SYSCALL3(__NR_execve, execve, const char *, filename, char **, args,
	      char **, envs);

DECL_SYSCALL3(__NR_update_screen, update_display, size_t, w, size_t, h, void *,
	      buffered_data);

DECL_SYSCALL1(__NR_signal_register, register_vars, void *, vars);
DECL_SYSCALL2(__NR_kill, kill, pid_t, pid, int, sig);
DECL_SYSCALL3(__NR_setitimer, setitimer, int, which, const struct itimerval *,
	      value, struct itimerval *, ovalue);

int printf(const char *fmt, ...);
int atexit(void (*function)(void));
void exit(uint32_t exitcode);