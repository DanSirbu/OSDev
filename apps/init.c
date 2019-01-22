#include "syscalls.h"
#include "main.h"

void main()
{
	unsigned int y = fork();
	if (y == 0) {
		printf("I'm a child process %x\n", y);
	} else {
		printf("I'm a parent process! Child pid: %d\n", y);
	}
}