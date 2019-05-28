#include "syscalls.h"

void testFunc()
{
	printf("Clone complete!\n");
	for (int x = 0; x < 10; x++) {
		printf("X is %d\n", x);
	}
}
int main()
{
	unsigned int y = fork();
	if (y == 0) {
		printf("I'm a child process %x\n", y);
		int x = fork();
		if (x == 0) {
			printf("I'm a super child\n");
			exec("/test");
		} else {
			printf("I'm just a normal child\n");
			//clone(testFunc);
			int res = exec("/testProgram.a");
			if (res == -1) {
				printf("ERROR: Exec testProgram failed\n");
			}
		}
	} else {
		printf("I'm a parent process! Child pid: %d\n", y);
	}

	return 0;
}