#include "types.h"
#include "serial.h"
#include "coraxstd.h"

#define NUM_SYSCALLS 71
//TODO find a better way to do this
char **syscall_names[NUM_SYSCALLS] = {
	"__NR_setup",   "__NR_exit",    "__NR_fork",	 "__NR_read",
	"__NR_write",   "__NR_open",    "__NR_close",	"__NR_waitpid",
	"__NR_creat",   "__NR_link",    "__NR_unlink",       "__NR_execve",
	"__NR_chdir",   "__NR_time",    "__NR_mknod",	"__NR_chmod",
	"__NR_chown",   "__NR_break",   "__NR_stat",	 "__NR_lseek",
	"__NR_getpid",  "__NR_mount",   "__NR_umount",       "__NR_setuid",
	"__NR_getuid",  "__NR_stime",   "__NR_ptrace",       "__NR_alarm",
	"__NR_fstat",   "__NR_pause",   "__NR_utime",	"__NR_stty",
	"__NR_gtty",    "__NR_access",  "__NR_nice",	 "__NR_ftime",
	"__NR_sync",    "__NR_kill",    "__NR_rename",       "__NR_mkdir",
	"__NR_rmdir",   "__NR_dup",     "__NR_pipe",	 "__NR_times",
	"__NR_prof",    "__NR_brk",     "__NR_setgid",       "__NR_getgid",
	"__NR_signal",  "__NR_geteuid", "__NR_getegid",      "__NR_acct",
	"__NR_phys",    "__NR_lock",    "__NR_ioctl",	"__NR_fcntl",
	"__NR_mpx",     "__NR_setpgid", "__NR_ulimit",       "__NR_uname",
	"__NR_umask",   "__NR_chroot",  "__NR_ustat",	"__NR_dup2",
	"__NR_getppid", "__NR_getpgrp", "__NR_setsid",       "__NR_sbrk",
	"__NR_clone",   "__NR_seek",    "__NR_update_screen"
};

char *getSyscallName(size_t number)
{
	if (number < NUM_SYSCALLS) {
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