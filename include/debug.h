#pragma once

char *getSyscallName(size_t number);
void dump_stack_trace(uint32_t *ebp);