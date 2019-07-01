#pragma once
#include "trap.h"

char *getSyscallName(size_t number);
void dump_stack_trace(uint32_t *ebp);
void dump_registers(int_regs_t *regs);