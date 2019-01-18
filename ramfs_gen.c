#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint32_t;

typedef struct file_info {
	char name[64];
	uint32_t start;
	uint32_t len;
} __attribute__((packed)) file_info_t;

int main(int argc1, char const *argv1[])
{
	//argv[0] = filename (ramfs_gen), we don't care
	//argv[1] = output file name
	//rest are programs
	int argc = argc1 - 2;
	char **argv = &argv1[2];

	file_info_t *fileInfos = malloc(argc * sizeof(file_info_t));

	uint32_t file_start = sizeof(uint32_t) + argc * sizeof(file_info_t);

	for (int x = 0; x < argc; x++) {
		FILE *stream = fopen(argv[x], "r");
		fseek(stream, 0, SEEK_END);
		fileInfos[x].len = ftell(stream);
		fileInfos[x].start = file_start;
		strcpy(fileInfos[x].name, argv[x]);

		file_start += fileInfos[x].len;
	}

	FILE *outFile = fopen(argv[1], "w");
	uint32_t numFiles = argc;

	//Write num files
	fwrite(&numFiles, sizeof(numFiles), 1, outFile);

	//Write Headers
	fwrite(fileInfos, (argc * sizeof(file_info_t)), 1, outFile);

	//Write files
	for (int x = 0; x < argc; x++) {
		FILE *file = fopen(argv[x], "r");
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
