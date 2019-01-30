#include "fs.h"
#include "vfs.h"
#include "string.h"
#include "kmalloc.h"

vfs_node_t *fs_root;

/*
 * Input: an absolute path
 * Ex. /home/media/Games/
 * 
 * Output string array
 */
char **tokenize(char *path)
{
	path = &path[1]; //Ignore /
	int len = strlen(path);

	//Count the number of /, excluding \/
	int numTokens = 0;
	char prevChar = ' ';
	for (int x = 0; x < len; x++) {
		if (path[x] == '/' && prevChar != '\\') {
			numTokens++;
		}
		prevChar = path[x];
	}
	if (path[len - 1] != '/') {
		numTokens++; //To account for the last one
	}

	char **tokens = kcalloc(sizeof(char *) * (numTokens + 1));

	int pos = 0;
	int count = 0;

	for (int i = 0; i < numTokens; i++) {
		count = 0;

		while (!(path[pos] == '/' && path[pos - 1] != '\\')) {
			if (path[pos] == '\0') {
				break;
			}
			pos++;
			count++;
		}

		tokens[i] = kmalloc(count + 1);
		memcpy(tokens[i], &path[pos - count], count);
		tokens[i][count] = '\0';

		pos++; //Skip the /
	}

	tokens[numTokens] = NULL;

	return tokens;
}

void mount(char *path, inode_t ino)
{
	//Ex. path = /home/media/test

	char **tokens = tokenize(path);
}
void umount(char *path, inode_t ino)
{
}

inode_t *dirlookup(inode_t *directory, char *name)
{
	/*assert(directory->type | FS_DIRECTORY && "Is not a directory");

	for (uint32_t offset = 0; offset < directory->size;
	     offset += BLOCK_SIZE) {
		dirent_t *dirents =
			bread(directory->dev, directory->ino, offset);

		for (uint32_t x = 0; x < (BLOCK_SIZE / sizeof(dirent_t)); x++) {
			if (strncmp(dirents[x].name, name, FS_NAME_MAX_LEN) ==
			    0) { //They are the same
				return iget(dirents[x].ino);
			}
		}
		brelease(dirents);
	}
	*/
	return NULL;
}
int vfs_read(file_t *file, uint8_t *buf, size_t count, size_t *offset)
{
	if (file->f_op->read) {
		return file->f_op->read(file, buf, count, offset);
	} else {
		return -1;
	}
}
