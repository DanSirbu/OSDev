#include "sys.h"

DEF_SYSCALL1(0, exit, int, exitcode)
DEF_SYSCALL0(1, fork)
DEF_SYSCALL3(2, write, int, fd, char *, buf, int, size)