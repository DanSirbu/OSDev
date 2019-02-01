#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

#define FS_FILE 0x01
#define FS_DIRECTORY 0x02

typedef struct {
	uint32_t ino;
	char name[64];
} __attribute__((packed)) ramfs_dirent_t;

typedef struct {
	uint32_t numDir;
	ramfs_dirent_t dirents[6];
} __attribute__((packed)) ramfs_dir_t;

typedef struct file_info {
	uint32_t ino;
	uint8_t type;
	uint32_t start;
	uint32_t len;
} __attribute__((packed)) file_info_t; //Inodes

int main(int argc1, char const *argv1[])
{
	//argv[0] = filename (ramfs_gen), we don't care
	//argv[1] = output file name
	//rest are programs
	int argc = argc1 - 2;
	char **argv = (char **)&argv1[2];

	if (argc > 6) {
		fprintf(stderr, "Can't have more than 6 files.");
	}

	uint32_t numFileInfos = argc + 1;
	file_info_t *fileInfos = malloc(numFileInfos * sizeof(file_info_t));

	uint32_t files_data_start =
		sizeof(uint32_t) + numFileInfos * sizeof(file_info_t);

	//INO 0 = tree root
	fileInfos[0].len = sizeof(ramfs_dir_t);
	fileInfos[0].start = files_data_start;
	fileInfos[0].type = FS_DIRECTORY;
	fileInfos[0].ino = 0;

	files_data_start += sizeof(ramfs_dir_t);
	for (int x = 1; x < numFileInfos; x++) {
		FILE *stream = fopen(argv[x - 1], "r");
		fseek(stream, 0, SEEK_END);
		fileInfos[x].len = ftell(stream);
		fileInfos[x].start = files_data_start;
		fileInfos[x].type = FS_FILE;
		fileInfos[x].ino = x;

		files_data_start += fileInfos[x].len;
	}

	//Directory "file"
	ramfs_dir_t *rootDir = malloc(sizeof(ramfs_dir_t));
	rootDir->numDir = argc;
	for (int x = 0; x < argc; x++) {
		rootDir->dirents[x].ino = x + 1;
		char tmp[64];
		strcpy(tmp, argv[x]);
		char *base = basename(tmp);
		//printf("Base: %s\n", base);
		strcpy(rootDir->dirents[x].name, base);
	}

	FILE *outFile = fopen(argv1[1], "w");

	//Write num inodes
	fwrite(&numFileInfos, sizeof(numFileInfos), 1, outFile);

	//Write inodes
	fwrite(fileInfos, ((argc + 1) * sizeof(file_info_t)), 1, outFile);

	//Write root dir file
	fwrite(rootDir, sizeof(ramfs_dir_t), 1, outFile);

	//Write files
	for (int x = 1; x < numFileInfos; x++) {
		FILE *file = fopen(argv[x - 1], "r");
		char *buf = malloc(fileInfos[x].len);
		fread(buf, 1, fileInfos[x].len, file);
		fwrite(buf, fileInfos[x].len, 1, outFile);
		fclose(file);

		free(buf);
	}

	fclose(outFile);
	free(fileInfos);
	return 0;
}
