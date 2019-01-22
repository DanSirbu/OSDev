#include "syscalls.h"
#include "main.h"

int main()
{
	unsigned int y = fork();
	if (y == 0) {
		printf("I'm a child process %x\n", y);
		int x = fork();
		if(x == 0) {
			printf("I'm a super child\n");
		} else {
			printf("I'm just a normal child\n");
		}
	} else {
		printf("I'm a parent process! Child pid: %d\n", y);
	}

	return 0;
}