#include "include/signal.h"

void (*signal(int sig, void (*func)(int)))(int)
{
	//TODO implement
	int xyz = sig + (int)func; //Attribute unused
	xyz++; //Attribute unused

	return 0;
}