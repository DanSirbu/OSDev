#include "fs.h"
#include "mmu.h"
#include "assert.h"
#include "ramfs.h"

/* Prototypes */
static inode_t *ramfs_find_child(struct inode *parent, char *name);
static inode_t *getInode(uint32_t ino);
static int ramfs_read(struct inode *node, void *buf, uint32_t offset,
		      uint32_t size);
static inode_t *ramfs_get_child(struct inode *parent, uint32_t index);
static int ramfs_mkdir(struct inode *inode, struct dentry *dentry);

ramfs_header_t *ramfs_header;

inode_operations_t ramfs_ops = { .find_child = ramfs_find_child,
				 .read = ramfs_read,
				 .get_child = ramfs_get_child,
				 .mkdir = ramfs_mkdir };

void initramfs(ramfs_header_t *diskRamFSHeader)
{
	//OTV = offset to virtual
#define RAMFS_OTV(addr) ((void *)(addr) + (size_t)diskRamFSHeader)
	/*******************************/
	//Convert file offset address to virtual address
	//By adding diskRamFSHeader
	/*******************************/
	diskRamFSHeader->inodes = RAMFS_OTV(diskRamFSHeader->inodes);
	for (uint32_t x = 0; x < diskRamFSHeader->numInodes; x++) {
		diskRamFSHeader->inodes[x].address =
			(uint32_t)RAMFS_OTV(diskRamFSHeader->inodes[x].address);

		//Fix directory pointers
		if (diskRamFSHeader->inodes[x].type == FS_DIRECTORY) {
			ramfs_dir_t *dirHeader =
				(void *)diskRamFSHeader->inodes[x].address;
			dirHeader->dirents = RAMFS_OTV(dirHeader->dirents);
		}
	}

	/*******************************/
	//Copy file metadata to the heap
	/*******************************/
	ramfs_header = kmalloc(sizeof(ramfs_header_t));
	memcpy(ramfs_header, diskRamFSHeader, sizeof(ramfs_header_t));

	size_t inodes_size = sizeof(ramfs_inode_t) * ramfs_header->max_inodes;
	ramfs_header->inodes = kmalloc(inodes_size);
	memcpy(ramfs_header->inodes, diskRamFSHeader->inodes, inodes_size);

	//Copy directories "file" to heap
	for (uint32_t x = 0; x < ramfs_header->numInodes; x++) {
		if (ramfs_header->inodes[x].type != FS_DIRECTORY) {
			continue;
		}
		ramfs_dir_t *dirHeader =
			(void *)diskRamFSHeader->inodes[x].address;
		void *newDirHeader = kmalloc(sizeof(ramfs_dir_t));
		assert(newDirHeader != NULL);
		memcpy(newDirHeader, dirHeader, sizeof(ramfs_dir_t));
		ramfs_header->inodes[x].address = newDirHeader;

		size_t direntsSize =
			sizeof(ramfs_dirent_t) * dirHeader->num_dirs;
		ramfs_dirent_t *dirents = kmalloc(direntsSize);
		memcpy(dirents, dirHeader->dirents, direntsSize);
		ramfs_dir_t *newDir = newDirHeader;
		newDir->dirents = dirents;
	}
}
inode_t *ramfs_getRoot()
{
	return getInode(0);
}

static inode_t *getInode(uint32_t ino)
{
	inode_t *inode = kcalloc(sizeof(inode_t));
	inode->ino = ino;
	inode->size = ramfs_header->inodes[ino].size;
	inode->type = ramfs_header->inodes[ino].type;
	inode->i_op = &ramfs_ops;

	return inode;
}

static inode_t *ramfs_find_child(struct inode *parent, char *name)
{
	if (ramfs_header->inodes[parent->ino].type != FS_DIRECTORY) {
		return NULL;
	}

	ramfs_dir_t *dir =
		(ramfs_dir_t *)ramfs_header->inodes[parent->ino].address;

	for (uint32_t x = 0; x < dir->num_dirs; x++) {
		if (strncmp(dir->dirents[x].name, name, FS_NAME_MAX_LEN) == 0) {
			return getInode(dir->dirents[x].ino);
		}
	}

	return NULL;
}
static inode_t *ramfs_get_child(struct inode *parent, uint32_t index)
{
	if (ramfs_header->inodes[parent->ino].type != FS_DIRECTORY) {
		return NULL;
	}
	ramfs_dir_t *dir =
		(ramfs_dir_t *)ramfs_header->inodes[parent->ino].address;
	if (index >= dir->num_dirs) {
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

static int ramfs_read(struct inode *node, void *buf, uint32_t offset,
		      uint32_t size)
{
	if (node->ino >= ramfs_header->numInodes) {
		return -1;
	}

	uint32_t file_start = ramfs_header->inodes[node->ino].address;

	//Restrict to file length
	if (offset + size > ramfs_header->inodes[node->ino].size) {
		size = ramfs_header->inodes[node->ino].size - offset;
	}
	file_start += offset;

	memcpy(buf, (void *)file_start, size);

	return size;
}

static int ramfs_mkdir(struct inode *inode, struct dentry *dentry)
{
	assert(inode == NULL); //inode must be null for now
	assert(dentry != NULL);

	assert(dentry->parent->ino < ramfs_header->numInodes); //Sanity check
	assert(ramfs_header->inodes[dentry->parent->ino].type == FS_DIRECTORY);

	//If the inodes array is full
	if (ramfs_header->numInodes == ramfs_header->max_inodes) {
		//Double the size every time
		size_t newMaxInodes = ramfs_header->numInodes * 2;
		void *newInodes =
			krealloc(ramfs_header->inodes,
				 sizeof(ramfs_inode_t) * newMaxInodes);
		if (newInodes == NULL) {
			return -1;
		}

		ramfs_header->inodes = newInodes;
		ramfs_header->max_inodes = newMaxInodes;
	}

	/*******************************/
	//Add directory as a child to dentry->parent
	/*******************************/
	ino_t parentIno = dentry->parent->ino;
	ramfs_dir_t *parentDirFile = ramfs_header->inodes[parentIno].address;

	/*START: Increase parent directory "file" to support one more entry */
	void *newDirents = krealloc(parentDirFile->dirents,
				    sizeof(ramfs_dirent_t) *
					    (parentDirFile->num_dirs + 1));
	if (newDirents == NULL) {
		return -1;
	}
	parentDirFile->dirents = newDirents;
	/*END*/

	//Create directory inode
	ramfs_inode_t childInode;
	childInode.ino = ramfs_header->numInodes;
	childInode.type = FS_DIRECTORY;
	childInode.size = 0;
	childInode.max_size = 0;

	//Create directory entry in parent
	ramfs_dirent_t childDirEntry;
	strncpy(childDirEntry.name, dentry->name, FS_NAME_MAX_LEN);
	childDirEntry.ino = childInode.ino;
	memcpy(&parentDirFile->dirents[parentDirFile->num_dirs], &childDirEntry,
	       sizeof(childDirEntry));
	parentDirFile->num_dirs++;

	//Create the new inode's "file"
	ramfs_dir_t *childDirFile = kmalloc(sizeof(ramfs_dir_t));
	memset(childDirFile, 0, sizeof(ramfs_dir_t));

	//Link it to the inode
	childInode.address = childDirFile;
	childInode.size = sizeof(ramfs_dir_t);
	childInode.max_size = childInode.size;

	//Add the inode to the inodes array
	memcpy(&ramfs_header->inodes[childInode.ino], &childInode,
	       sizeof(childInode));
	ramfs_header->numInodes++;

	return 0;
}