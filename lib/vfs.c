#include "fs.h"

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
