#include "coraxstd.h"
#include "signal.h"
#include "assert.h"

#define sa_handler _signal_handlers._handler
#define sa_sigaction _signal_handlers._sigaction

union sigval {
	int sival_int;
	void *sival_ptr;
};

struct sigevent {
	int sigev_notify;
	int sigev_signo;
	union sigval sigev_value;
};

typedef struct {
	int si_signo;
	int si_code;
	union sigval si_value;
} siginfo_t;

typedef unsigned long sigset_t;
typedef void (*_sig_func_ptr)(int);

struct sigaction {
	int sa_flags;
	sigset_t sa_mask;
	union {
		_sig_func_ptr _handler;
		void (*_sigaction)(int, siginfo_t *, void *);
	} _signal_handlers;
};

void (*signal_handlers[SIGRTMAX])(int);

pid_t getpid()
{
	return call_getpid();
}
int sigaction(int sig, const struct sigaction *restrict act,
	      struct sigaction *restrict oact)
{
	fprintf(stderr, "IMPLEMENT: SIGACTION\n");

	return 0;
}
int raise(int sig)
{
	return kill(getpid(), sig);
}

void (*signal(int sig, void (*func)(int)))(int)
{
	assert(sig >= 0);
	signal_handlers[sig] = func;
}

void SignalReceived(int signum)
{
	if (signum > SIGRTMAX || signum < 0) {
		printf("Error: signum %d is invalid\n", signum);
		return;
	}

	if (signal_handlers[signum] != NULL) {
		(*signal_handlers[signum])(signum);
	}
}