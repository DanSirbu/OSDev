#include "syscalls.h"
#include "main.h"

void testFunc()
{
	printf("Clone complete!\n");
	for (int x = 0; x < 10; x++) {
		printf("X is %d\n", x);
	}
	while (1) //Wait since right now exiting a thread exits the process
		;
}
int main()
{
	/*unsigned int y = fork();
	if (y == 0) {
		printf("I'm a child process %x\n", y);
		int x = fork();
		if (x == 0) {
			printf("I'm a super child\n");
			exec("/test");
		} else {
			printf("I'm just a normal child\n");
			//clone(testFunc);
		}
	} else {
		printf("I'm a parent process! Child pid: %d\n", y);
	}*/
	clone(testFunc);
	while (1) //Wait since right now exiting a thread exits the process
		;

	return 0;
}