#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h>

#include "ramfs.h"

long getFileSize(const char *file)
{
	FILE *stream = fopen(file, "r");
	fseek(stream, 0, SEEK_END);
	long size = ftell(stream);
	fclose(stream);

	return size;
}

int main(int argc, char const *argv[])
{
	if (argc == 1) {
		fprintf(stderr, "%s filename\n", argv[0]);
		exit(-1);
	}
	const char *filename = argv[1];
	int fd = open(filename, O_RDONLY);

	void *mapped_data =
		mmap(NULL, (size_t)getFileSize(filename),
		     PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_POPULATE, fd, 0);

	if (mapped_data == NULL) {
		fprintf(stderr, "%s\n", "Unable to map data");
		exit(-1);
	}
	ramfs_header_t *ramfs_header = mapped_data;
	printf("NumInodes: %d\n", ramfs_header->numInodes);

	void *newInodeLocation =
		(void *)ramfs_header->inodes + (size_t)mapped_data;
	ramfs_header->inodes = newInodeLocation;

	for (uint32_t x = 0; x < ramfs_header->numInodes; x++) {
		printf("Ino %d\n", ramfs_header->inodes[x].ino);
		ramfs_header->inodes[x].address += (size_t)mapped_data;
	}
	ramfs_dir_t *dirs = (void *)ramfs_header->inodes[0].address;
	printf("NumFiles: %d\n", dirs->num_dirs);
	dirs->dirents = ((void *)dirs->dirents) + (size_t)mapped_data;
	for (uint32_t x = 0; x < dirs->num_dirs; x++) {
		printf("Ino: %d Name: %s\n", dirs->dirents[x].ino,
		       dirs->dirents[x].name);
	}

	close(fd);
}