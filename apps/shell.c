#include "syscalls.h"
#include "signal.h"
#include "coraxstd.h"
#include "implementme.h"
#include "malloc.h"
#include "string.h"
#include "assert.h"
#include "keyboard_map.h"

extern char **environ;

char *read_line()
{
	int buf_size = 1024;
	char *buf = malloc(buf_size);
	int bufIndex = 0;
	while (true) {
		uint8_t character = getc(stdin);
		if (character == KEY_PRESSED) {
			getc(stdin);
			continue;
		}

		if (character == KEY_RELEASED) {
			character = getc(stdin);
		}

		switch (character) {
		case KEY_RETURN:
			character = '\n';
			break;
		case KEY_SLASH:
			character = '/';
			break;
		case KEY_BACKSLASH:
			character = '\\';
			break;
		case KEY_DELETE:
			character = '\b';
			break;
		case KEY_SPACE:
			character = ' ';
			break;
		}

		//Grow buffer if full
		if (bufIndex == buf_size) {
			buf_size *= 2;
			buf = realloc(buf, buf_size);
		}

		if (character == '\n') {
			buf[bufIndex] = '\0';
			printf("%s", "\n");
			return buf;
		} else if (character == '\b') {
			if (bufIndex > 0) {
				bufIndex--;
			} else {
				continue;
			}
		} else {
			buf[bufIndex] = character;
			bufIndex++;
		}

		char charBuf[2];
		charBuf[0] = character;
		charBuf[1] = '\0';
		printf("%s", charBuf);
	}
}
void run_process(char *command)
{
	char **args = tokenize(command, " ");
	if (access(args[0], 0) != 0) {
		printf("%s: command not found\n", args[0]);
		free_arr(args);
		return;
	}

	int pid = fork();

	/*int fds[2];
    pipe(fds);
    Ignore for now
    */

	if (pid == 0) { //Child
		execve(args[0], args, environ);

		exit(-1);
	}

	free_arr(args);
	assert(waitpid(pid, NULL, NULL) == 0);
}

void clear()
{
	for (int x = 0; x < 60; x++) {
		printf("%*s", 80, " ");
	}
}
int main(int argc, char *args[])
{
	int buf[1024];

	while (true) {
		printf("corax$");
		char *line = read_line();
		if (strncasecmp(line, "clear", sizeof("clear")) == 0) {
			clear();
		} else if (strncasecmp(line, "help", sizeof("help")) == 0) {
			puts("\n");
			puts("Welcome to coraxOS!\n");
			puts("help - display this message\n");
			puts("clear - clear the terminal\n");
			puts("filename [arguments] - run program with the specified arguments\n");
			puts("\n");
		} else if (strlen(line) == 0) {
			//Do nothing
		} else {
			run_process(line);
		}

		free(line);
	}
}