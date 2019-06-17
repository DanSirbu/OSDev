#include "signal.h"
#include "sys/types.h"

int raise(int sig)
{
	return kill(getpid(), sig);
}
void (*signal(int sig, void (*func)(int)))(int)
{
	//TODO implement
	int xyz = sig + (int)func; //Attribute unused
	xyz++; //Attribute unused

	return 0;
}