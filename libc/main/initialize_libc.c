#include "syscalls.h"
#include "coraxstd.h"

extern void OnSignal(void);
extern int errno;
extern void run_clone(void (*fn)());

void initialize_libc()
{
	//TODO syscall register SignalReceived
	struct userspace_vars vars;

	vars.errno_addr = &errno;
	vars.SignalHandler = OnSignal;
	vars.clone_func_caller = run_clone;
	register_vars(&vars);
}