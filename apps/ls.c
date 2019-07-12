#include "syscalls.h"
#include "signal.h"
#include "coraxstd.h"
#include "implementme.h"
#include "malloc.h"
#include "string.h"
#include "assert.h"
#include "keyboard_map.h"
#include "dirent.h"
#include "fs_info.h"

int main(int argc, char *args[])
{
	char *directoryToOpen = args[1];
	if (argc < 2) {
		directoryToOpen = "/";
	}
	int fd = open(directoryToOpen, 0);
	if (fd < 0) {
		printf("ls: cannot access '%s': No such file or directory\n",
		       directoryToOpen);
		return -1;
	}
	struct stat stat;
	fstat(fd, &stat);
	if (!(stat.st_type & FS_DIRECTORY)) {
		printf("Error: not a directory\n");
		return -1;
	}

	dir_header_t dir_header;

	read(fd, &dir_header, sizeof(dir_header));

	size_t direntsSize = sizeof(dir_dirent_t) * dir_header.num_dirs;
	dir_dirent_t *dirents = malloc(direntsSize);
	getdents(fd, dirents, dir_header.num_dirs);

	for (uint32_t i = 0; i < dir_header.num_dirs; i++) {
		puts(dirents[i].name);
		puts("\n");
	}

	free(dirents);
	close(fd);
}