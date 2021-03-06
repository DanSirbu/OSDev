#include "syscalls.h"
#include "signal.h"
#include "coraxstd.h"
#include "implementme.h"
#include "assert.h"

/*void testFunc()
{
	printf("Clone complete!\n");
	for (int x = 0; x < 10; x++) {
		printf("X is %d\n", x);
	}
}
void alarm_handler()
{
	struct timeval p;
	gettimeofday(&p, NULL);
	printf("Alarm triggered at: %d\n", p.tv_sec);
}*/
extern char **environ;
int main(int argc, char *args[])
{
	/*signal(SIGALRM, alarm_handler);

#define SDL_alarm_interval 1000
	struct itimerval timer;

	timer.it_value.tv_sec = (SDL_alarm_interval / 1000);
	timer.it_value.tv_usec = (SDL_alarm_interval % 1000) * 1000;
	timer.it_interval.tv_sec = (SDL_alarm_interval / 1000);
	timer.it_interval.tv_usec = (SDL_alarm_interval % 1000) * 1000;
	setitimer(ITIMER_REAL, &timer, NULL);

	printf("Before raise\n");
	raise(SIGALRM);
	printf("After raise\n");
	while (1) {
	};

	unsigned int y = fork();
	char *args2[] = { "/test", NULL };
	if (y == 0) {
		printf("I'm a child process %x\n", y);
		int x = fork();
		if (x == 0) {
			printf("I'm a super child\n");
			execve("/test", args, environ);
		} else {
			printf("I'm just a normal child\n");
			//clone(testFunc);
			int res = execve("/testProgram.a", args, environ);
			if (res == -1) {
				printf("ERROR: Exec testProgram failed\n");
			}
		}
	} else {
		printf("I'm a parent process! Child pid: %d\n", y);
	}*
	*/
	FILE *readFile = fopen("/dev/keyboard", 0);
	//int writeFD = open("/dev/serial", 0);
	int writeFD = open("/dev/screen", 0);
	int errFD = open("/dev/null", 0);

	/*while (true) {
		int character = getc(readFile);
		write(writeFD, (void *)&character, 1);
	}*/

	/*char *args1[] = { "/prboom", NULL };
	char **envs = (char **)environ;
	execve("/prboom", args1, envs);*/

	char *args1[] = { "/shell", NULL };
	char **envs = (char **)environ;
	execve("/shell", args1, envs);

	return -1;
}