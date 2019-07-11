#pragma once
#include "trap.h"
#include "multiboot.h"

void parse_elf_sections(multiboot_elf_section_header_table_t *sectionsHeader,
			size_t *max_address);
char *getSyscallName(size_t number);
void dump_stack_trace(uint32_t *ebp);
void dump_registers(int_regs_t *regs);