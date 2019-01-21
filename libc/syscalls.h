#define DECL_SYSCALL0(unused, name) int name();
#define DECL_SYSCALL1(unused, name, P1, p1) int name(P1 p1);
#define DECL_SYSCALL2(unused, name, P1, p1, P2, p2) int name(P1 p1, P2 p2);
#define DECL_SYSCALL3(unused, name, P1, p1, P2, p2, P3, p3) int name(P1 p1, P2 p2, P3 p3);

DECL_SYSCALL1(0, exit, int, exitcode)
DECL_SYSCALL0(1, fork)
DECL_SYSCALL3(2, write, int, fd, char*, buf, int, size)