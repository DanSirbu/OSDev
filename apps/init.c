#include "sys/types.h"
#include "syscalls.h"

void testFunc()
{
	printf("Clone complete!\n");
	for (int x = 0; x < 10; x++) {
		printf("X is %d\n", x);
	}
}

extern char **environ;
int main(int argc, char *args[])
{
	/*unsigned int y = fork();
	if (y == 0) {
		printf("I'm a child process %x\n", y);
		int x = fork();
		if (x == 0) {
			printf("I'm a super child\n");
			execve("/test");
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
	}*/

	char *args1[] = { "/prboom", "-nodraw", "-nosound", NULL };
	char **envs = (char **)environ;
	execve("/prboom", args1, envs);
	//execve("/testApp", args1, envs);

	return -1;
}