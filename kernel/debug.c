#include "sys/types.h"
#include "serial.h"
#include "coraxstd.h"

extern size_t SYSCALL_NAMES_SIZE;
extern char *syscall_names[];

char *getSyscallName(size_t number)
{
	if (number < SYSCALL_NAMES_SIZE) {
		return syscall_names[number];
	}
	return NULL;
}
uint32_t get_function(uint32_t ip)
{
	/*for (uint32_t x = 0; x < (sizeof(entries) / sizeof(entries[0])) - 1;
	     x++) {
		if (entries[x].address <= ip && entries[x + 1].address >= ip) {
			return entries[x].address;
		}
	}
	return entries[0].address; //TODO, sensible default*/
	return 0;
}
void get_func_info(uint32_t addr, char **name, char **file)
{
	/*for (uint32_t x = 0; x < (sizeof(entries) / sizeof(entries[0])); x++) {
		if (entries[x].address == addr) {
			*name = entries[x].func_name;
			*file = entries[x].file;
			break;
		}
	}*/
	//return 0;
}

extern uint32_t *get_ebp();
void dump_stack_trace()
{
	uint32_t *ebp = get_ebp();

	debug_print("Backtrace:\n");

	for (; ebp != 0; ebp = (uint32_t *)*ebp) {
		uint32_t retIP = *(ebp + 1);

		if (retIP == 0) { //We finished. See boot.asm where we push 0
			break;
		}

		uint32_t func_addr = get_function(retIP);

		char *func_name;
		char *func_file;

		get_func_info(func_addr, &func_name, &func_file);

		debug_print("%d: %s in %s\n", 1, func_name, func_file);
	}
}