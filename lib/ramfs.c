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

void initramfs(vptr_t location)
{
	ramfs_ops = kcalloc(sizeof(inode_operations_t));
	ramfs_ops->find_child = ramfs_find_child;

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
typedef struct {
	uint32_t ino;
	char name[FS_NAME_MAX_LEN];
} __attribute__((packed)) ramfs_dirent_t;

typedef struct {
	uint32_t numDir;
	ramfs_dirent_t dirents[6];
} __attribute__((packed)) ramfs_dir_t;

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
}

uint32_t ramfs_read(file_t *node, uint8_t *buf, uint32_t size, uint32_t *offset)
{
	void *file_start = headers[node->ino].start;


    if(offset + size > headers[node->ino].len) {
        size = headers[node->ino].len - offset;
    }
    file_start += offset;

    memcpy(buf, file_start, size);

	return -1;
}
*/