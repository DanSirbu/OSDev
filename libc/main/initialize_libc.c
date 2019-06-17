#include "syscalls.h"
#include "coraxstd.h"

extern void OnSignal(void);
extern int errno;
void initialize_libc()
{
	//TODO syscall register SignalReceived
	struct userspace_vars vars;

	vars.errno_addr = &errno;
	vars.SignalHandler = OnSignal;
	register_vars(&vars);
}