#pragma once

#include "sys/types.h"
#include "serial.h"

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21

#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1

#define TRAP_PAGE_FAULT 14
#define SYSCALL_NO 0x80

#define PIC_REMAPPED_START 32
#define PIC_REMAPPED_END (PIC_REMAPPED_START + 15)
#define TRAP_TIMER 32
#define TRAP_KEYBOARD (32 + 1)

typedef struct {
	uint32_t gs, fs, es, ds; //Segments pushed
	uint32_t edi, esi, ebp, unused, ebx, edx, ecx,
		eax; //pushad, note: unused is actually esp, but we don't want it to be modifiable
	uint32_t interrupt_no, error_code; //Pushed by assembly code
	uint32_t eip, cs, eflags, useresp, ss; //Pushed by CPU
} __attribute__((packed)) int_regs_t;

typedef void (*isr_handler_t)(int_regs_t *);

void register_isr_handler(int interrupt_no, isr_handler_t handler);

void interrupt_handler(int_regs_t regs);