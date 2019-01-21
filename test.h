# 1 "libc/syscall.h"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "libc/syscall.h"
# 1 "libc/sys.h" 1
       
# 32 "libc/sys.h"
static inline int syscall(int num, int p1, int p2, int p3, int p4, int p5) {
    int ret;
    asm ("push %%ebx; push %%esi; push %%edi;     movl %1, %%eax;     movl %2, %%ebx;     movl %3, %%ecx;     movl %4, %%edx;     movl %5, %%esi;     movl %6, %%edi;     int $0x80;     movl %[retval], %%eax;     pop %%edi; pop %%esi; pop %%ebx"
# 44 "libc/sys.h"
    : [retval] "=r" (ret)
    : "0" (num), "g" (p1), "g"(p2), "g"(p3), "g"(p4), "g"(p5)
    : "ebx", "esi", "edi", "eax");

    return ret;
}
# 2 "libc/syscall.h" 2

int syscall_exit(int exitcode) { return syscall(0, (int) exitcode, 0, 0, 0, 0); }
