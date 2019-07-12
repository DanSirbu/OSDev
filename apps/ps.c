#include "syscalls.h"
#include "signal.h"
#include "coraxstd.h"
#include "implementme.h"
#include "malloc.h"
#include "string.h"
#include "assert.h"
#include "keyboard_map.h"
#include "dirent.h"
#include "string.h"

int main(int argc, char *args[])
{
	char *directoryToOpen = "/proc";
	int fd = open(directoryToOpen, 0);
	dir_header_t dir_header;

	read(fd, &dir_header, sizeof(dir_header));

	size_t direntsSize = sizeof(dir_dirent_t) * dir_header.num_dirs;
	dir_dirent_t *dirents = malloc(direntsSize);
	getdents(fd, dirents, dir_header.num_dirs);

	for (uint32_t i = 0; i < dir_header.num_dirs; i++) {
		char *tmpPath =
			malloc(strlen("/proc/") + strlen(dirents[i].name) + 1);
		strconcat(tmpPath, "/proc/", dirents[i].name);
		int procFD = open(tmpPath, 0);
		free(tmpPath);

		char processContents[1024];
		read(procFD, processContents, 1024);
		puts(processContents);
		printf("\n");

		close(procFD);
	}

	free(dirents);
	close(fd);
}