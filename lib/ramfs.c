#include "fs.h"
#include "mmu.h"
#include "assert.h"
#include "ramfs.h"

typedef struct file_info {
	uint32_t ino;
	uint8_t type;
	uint32_t start;
	uint32_t len;
} __attribute__((packed)) file_info_t; //Inodes

file_info_t *headers;
uint32_t numInodes;

inode_operations_t *ramfs_ops;

inode_t *ramfs_find_child(struct inode *parent, char *name);
inode_t *getInode(uint32_t ino);
int ramfs_read(struct inode *node, void *buf, uint32_t offset, uint32_t size);
inode_t *ramfs_get_child(struct inode *parent, uint32_t index);

void initramfs(vptr_t location)
{
	ramfs_ops = kcalloc(sizeof(inode_operations_t));
	ramfs_ops->find_child = ramfs_find_child;
	ramfs_ops->read = ramfs_read;
	ramfs_ops->get_child = ramfs_get_child;

	headers = (void *)location + sizeof(uint32_t);
	numInodes = *((uint32_t *)location);

	for (uint32_t x = 0; x < numInodes; x++) {
		headers[x].start += location; //Transform to virtual
	}
}
inode_t *ramfs_getRoot()
{
	return getInode(0);
}

inode_t *getInode(uint32_t ino)
{
	inode_t *inode = kcalloc(sizeof(inode_t));
	inode->ino = ino;
	inode->size = headers[ino].len;
	inode->type = headers[ino].type;
	inode->i_op = ramfs_ops;

	return inode;
}

inode_t *ramfs_find_child(struct inode *parent, char *name)
{
	if (headers[parent->ino].type != FS_DIRECTORY) {
		return NULL;
	}

	ramfs_dir_t *dir = (ramfs_dir_t *)headers[parent->ino].start;

	for (uint32_t x = 0; x < dir->numDir; x++) {
		if (strncmp(dir->dirents[x].name, name, FS_NAME_MAX_LEN) == 0) {
			return getInode(dir->dirents[x].ino);
		}
	}

	return NULL;
}
inode_t *ramfs_get_child(struct inode *parent, uint32_t index)
{
	if (headers[parent->ino].type != FS_DIRECTORY) {
		return NULL;
	}

	ramfs_dir_t *dir = (ramfs_dir_t *)headers[parent->ino].start;
	if (index >= dir->numDir) {
		return NULL;
	}

	return getInode(dir->dirents[index].ino);
}

/*int (*close)(struct inode);
int (*read)(struct inode, void *, uint32_t offset, uint32_t size);
int (*write)(struct inode, void *, uint32_t offset, uint32_t size);
*/

/*dirent_t *initrd_readdir(fs_node_t *node, uint32_t index)
{
	assert(node == ram_root);
	if (index > numfiles - 1) {
		return NULL;
	}

	strcpy(dirent.name, headers[index].name);
	dirent.ino = index;

	return &dirent;
}*/

int ramfs_read(struct inode *node, void *buf, uint32_t offset, uint32_t size)
{
	uint32_t file_start = headers[node->ino].start;
	if (node->ino >= numInodes) {
		return -1;
	}

	//Restrict to file length
	if (offset + size > headers[node->ino].len) {
		size = headers[node->ino].len - offset;
	}
	file_start += offset;

	memcpy(buf, (void *)file_start, size);

	return size;
}