#pragma once

//EAX, ECX, and EDX are caller saved
//ebp is saved by the function prologue
//%eax for syscall_number. parameters: %ebx, %ecx, %edx, %esi, %edi, %ebp
//so we have to save ebx, esi, edi
#define DEF_SYSCALL0(fn, name)                                                 \
	int name()                                                             \
	{                                                                      \
		return syscall(fn, 0, 0, 0, 0, 0);                             \
	}
#define DEF_SYSCALL1(fn, name, P1, p1)                                         \
	int name(P1 p1)                                                        \
	{                                                                      \
		return syscall(fn, (int)p1, 0, 0, 0, 0);                       \
	}
#define DEF_SYSCALL2(fn, name, P1, p1, P2, p2)                                 \
	int name(P1 p1, P2 p2)                                                 \
	{                                                                      \
		return syscall(fn, (int)p1, (int)p2, 0, 0, 0);                 \
	}
#define DEF_SYSCALL3(fn, name, P1, p1, P2, p2, P3, p3)                         \
	int name(P1 p1, P2 p2, P3 p3)                                          \
	{                                                                      \
		return syscall(fn, (int)p1, (int)p2, (int)p3, 0, 0);           \
	}
/*#define DEF_SYSCALL4(fn, name, p1, p2, p3, p4) \
    inline int syscall_##name() { \
        return syscall(fn, (int) p1, (int) p2, (int) p3, (int) p4, 0); \
    }
#define DEF_SYSCALL5(fn, name, p1, p2, p3, p4, p5) \
    inline int syscall_##name() { \
        return syscall(fn, (int) p1, (int) p2, (int) p3, (int) p4, (int) p5); \
    }
*/
static inline int syscall(int num, int p1, int p2, int p3, int p4, int p5)
{
	int ret;
	asm("push %%ebx; push %%esi; push %%edi; \
    movl %1, %%eax; \
    movl %2, %%ebx; \
    movl %3, %%ecx; \
    movl %4, %%edx; \
    movl %5, %%esi; \
    movl %6, %%edi; \
    int $0x80; \
    movl %[retval], %%eax; \
    pop %%edi; pop %%esi; pop %%ebx"
	    : [retval] "=r"(ret)
	    : "0"(num), "g"(p1), "g"(p2), "g"(p3), "g"(p4), "g"(p5)
	    : "ebx", "esi", "edi", "eax");

	return ret;
}