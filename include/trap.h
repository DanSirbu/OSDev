#pragma once

#include "types.h"
#include "serial.h"

#define TRAP_PAGE_FAULT 14
#define SYSCALL_NO 0x80

#define TRAP_TIMER 32
#define TRAP_KEYBOARD (32 + 1)

typedef struct {
	uint32_t gs, fs, es, ds; //Segments pushed
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; //pushad
	uint32_t interrupt_no, error_code; //Pushed by assembly code
	uint32_t eip, cs, eflags, useresp, ss; //Pushed by CPU
} __attribute__((packed)) int_regs_t;

typedef void (*isr_handler_t)(int_regs_t *);

void register_isr_handler(int interrupt_no, isr_handler_t handler);

void interrupt_handler(int_regs_t regs);