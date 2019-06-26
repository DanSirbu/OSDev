#include "sys/types.h"
#include "serial.h"
#include "multiboot.h"
#include "elf.h"
#include "mmu.h"
#include "trap.h"
#include "coraxstd.h"

extern size_t SYSCALL_NAMES_SIZE;
extern char *syscall_names[];

Elf32_Addr stringTable;
Elf32_Sym *symTbl;
size_t symTblNumEntries;

char *getSyscallName(size_t number)
{
	if (number < SYSCALL_NAMES_SIZE) {
		return syscall_names[number];
	}
	return NULL;
}
void get_func_info(uint32_t addr, char **name, char **file)
{
	for (size_t i = 0; i < symTblNumEntries; i++) {
		if (symTbl[i].st_name == STN_UNDEF) {
			continue;
		}
		if (ELF32_ST_TYPE(symTbl[i].st_info) == STT_FUNC) {
			/*debug_print("Func: %s at 0x%x-0x%x\n",
				    stringTable + symTbl[i].st_name,
				    symTbl[i].st_value,
				    symTbl[i].st_value + symTbl[i].st_size);*/
			size_t lowAddress = symTbl[i].st_value;
			size_t highAddress = lowAddress + symTbl[i].st_size;

			if (addr >= lowAddress && addr < highAddress) {
				*name = stringTable + symTbl[i].st_name;
				*file = "";
			}
		}
	}
}

extern uint32_t *get_ebp();
void dump_stack_trace(uint32_t *ebp)
{
	if (ebp == NULL) {
		ebp = get_ebp();
	}
	debug_print("Backtrace:\n");
	int count = 0;
	for (; ebp != 0; ebp = (uint32_t *)*ebp) {
		uint32_t retIP = *(ebp + 1);

		if (retIP == 0) { //We finished. See boot.asm where we push 0
			break;
		}
		char *func_name;
		char *func_file;

		get_func_info(retIP, &func_name, &func_file);

		debug_print("%d: %s\n", count, func_name, func_file);
		count++;

		if (strncmp(func_name, "interrupt_handler",
			    sizeof("interrupt_handler")) == 0) {
			break; //Stop stack trace when we get to the interrupt handler
		}
	}
}

void dump_registers(int_regs_t *regs)
{
	print(LOG_ERROR, "Registers during interrupt:\n");
	print(LOG_ERROR, "eax=0x%x  ebx=0x%x\n", regs->eax, regs->ebx);
	print(LOG_ERROR, "ecx=0x%x  edx=0x%x\n", regs->ecx, regs->edx);
	print(LOG_ERROR, "edi=0x%x  esi=0x%x\n", regs->edi, regs->esi);
	print(LOG_ERROR, "ebp=0x%x  esp=0x%x\n", regs->ebp, regs->unused);
	print(LOG_ERROR, "eflags=0x%x       \n", regs->eflags);
	print(LOG_ERROR, "eip=0x%x          \n", regs->eip);
	print(LOG_ERROR, "\n");
	print(LOG_ERROR, "usereip=0x%x      \n", regs->useresp);
}

void parse_elf_sections(multiboot_elf_section_header_table_t *sectionsHeader,
			size_t *max_address)
{
	assert(sectionsHeader->num != 0);

	size_t sections_end = sectionsHeader->addr +
			      (sectionsHeader->num * sectionsHeader->size);
	if (max_address != NULL) {
		*max_address = sections_end;
	}
	debug_print("Section start: 0x%x\n", sectionsHeader->addr);
	debug_print("Section end: 0x%x\n", sections_end);

	Elf32_Shdr *sections = KERN_P2V(sectionsHeader->addr);
	Elf32_Addr sectionStringTable =
		KERN_P2V(sections[sectionsHeader->shndx].sh_addr);

	for (size_t x = 0; x < sectionsHeader->num; x++) {
		if (sections[x].sh_type == SHT_NULL) { //NULL entry
			continue;
		}
		if (sections[x].sh_type == SHT_STRTAB &&
		    strncmp((char *)(sectionStringTable + sections[x].sh_name),
			    ".strtab", sizeof(".strtab")) == 0) {
			stringTable = (Elf32_Addr)KERN_P2V(sections[x].sh_addr);
		}
	}

	for (size_t x = 0; x < sectionsHeader->num; x++) {
		if (sections[x].sh_type == SHT_NULL) { //NULL entry
			continue;
		}
		if (sections[x].sh_type == SHT_SYMTAB && symTbl == NULL) {
			assert(sections[x].sh_addr != NULL);
			symTbl = KERN_P2V(sections[x].sh_addr);
			symTblNumEntries =
				sections[x].sh_size / sections[x].sh_entsize;
		}
		debug_print("Section %s, %x at 0x%x\n",
			    sectionStringTable + sections[x].sh_name,
			    sections[x].sh_type, sections[x].sh_addr);
	}
}