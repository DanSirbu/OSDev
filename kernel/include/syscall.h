#pragma once

void syscalls_install();

#define DEF_SYSCALL0(no, fn)                                                   \
	case no:                                                               \
		regs->eax = sys_##fn();                                        \
		break;

#define DEF_SYSCALL1(no, fn, P1, p1)                                           \
	case no:                                                               \
		regs->eax = sys_##fn((P1)regs->ebx);                           \
		break;

#define DEF_SYSCALL2(no, fn, P1, p1, P2, p2)                                   \
	case no:                                                               \
		regs->eax = sys_##fn((P1)regs->ebx, (P2)regs->ecx);            \
		break;

#define DEF_SYSCALL3(no, fn, P1, p1, P2, p2, P3, p3)                           \
	case no:                                                               \
		regs->eax =                                                    \
			sys_##fn((P1)regs->ebx, (P2)regs->ecx, (P3)regs->edx); \
		break;

void set_userspace_errno(int errno);