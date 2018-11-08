#pragma once

#include "types.h"
#include "serial.h"

#define TRAP_PAGE_FAULT 14

#define TRAP_TIMER 32
#define TRAP_KEYBOARD (32 + 1)

void register_handler(int interrupt_no, void *handler);

void interrupt_handler(u32 cr2, u32 edi, u32 esi, u32 ebp, u32 esp, u32 ebx,
		       u32 edx, u32 ecx, u32 eax, const u32 interrupt_no,
		       u32 error_code, size_t eip);