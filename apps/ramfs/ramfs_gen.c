#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <stdint.h>

#include "ramfs.h"

/*
File format:
header
inodes
files
 */

long getFileSize(char *file)
{
	FILE *stream = fopen(file, "r");
	fseek(stream, 0, SEEK_END);
	long size = ftell(stream);
	fclose(stream);

	return size;
}
int main(int argc, char const *argv[])
{
	//argv[0] = filename (ramfs_gen), we don't care
	//argv[1] = output file name
	//rest are programs
	int numFiles = argc - 2;
	char **files = (char **)&argv[2];

	size_t fileOffset = 0;

	size_t numInodes = numFiles + 1; //numfiles + root inode

	ramfs_header_t *ramfs_header = malloc(sizeof(ramfs_header_t));
	fileOffset += sizeof(ramfs_header_t); //File: add header offset
	ramfs_header->numInodes = numInodes;
	ramfs_header->max_inodes = numInodes;
	ramfs_header->inodes = (void *)fileOffset;
	ramfs_inode_t *inodes = malloc(sizeof(ramfs_inode_t) * numInodes);
	fileOffset +=
		sizeof(ramfs_inode_t) * numInodes; //File: add inodes offset

	int nextInode = 1;
	for (int x = 0; x < numFiles; x++) {
		long fileSize = getFileSize(files[x]);
		inodes[nextInode].size = fileSize;
		inodes[nextInode].max_size = fileSize;
		inodes[nextInode].read_only = 1;
		inodes[nextInode].address = fileOffset;
		inodes[nextInode].type = FS_FILE;
		inodes[nextInode].ino = nextInode;

		nextInode++;
		fileOffset += fileSize; //File: add individual file offset
	}
	//INO 0 = tree root, place the inode contents last in the file
	inodes[0].size = fileOffset;
	inodes[0].max_size = fileOffset;
	inodes[0].read_only = 1;
	inodes[0].address = fileOffset;
	inodes[0].type = FS_DIRECTORY;
	inodes[0].ino = 0;

	//Directory "file"
	ramfs_dir_t *rootDir = malloc(sizeof(ramfs_dir_t));
	rootDir->dirents =
		(void *)(fileOffset + sizeof(ramfs_dir_t)); //Add pointer offset
	rootDir->num_dirs = numFiles;
	ramfs_dirent_t *dirents = malloc(sizeof(ramfs_dirent_t) * numFiles);

	printf("RAMFS Files:");
	for (int x = 0; x < numFiles; x++) {
		dirents[x].ino = x + 1;
		char tmp[64];
		strcpy(tmp, files[x]);
		char *base = basename(tmp);
		printf(" %s", base); //Print out filename to terminal
		strcpy(dirents[x].name, base);
	}
	printf("\n");

	//Actually create the ramfs file
	const char *outFileName = argv[1];
	FILE *outFile = fopen(outFileName, "w");

	//Write header
	fwrite(ramfs_header, sizeof(ramfs_header_t), 1, outFile);

	//Write inodes
	fwrite(inodes, sizeof(ramfs_inode_t) * numInodes, 1, outFile);

	//Write files
	for (int x = 0; x < numFiles; x++) {
		FILE *file = fopen(files[x], "r");
		uint32_t filesize = inodes[x + 1].size;
		char *buf = malloc(filesize);
		fread(buf, 1, filesize, file);
		fwrite(buf, filesize, 1, outFile);
		fclose(file);

		free(buf);
	}

	//Write root dir file
	fwrite(rootDir, sizeof(ramfs_dir_t), 1, outFile);
	fwrite(dirents, sizeof(ramfs_dirent_t) * numFiles, 1, outFile);

	fclose(outFile);

	free(inodes);
	free(ramfs_header);
	return 0;
}
