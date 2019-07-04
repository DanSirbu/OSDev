#include "fs.h"
#include "vfs.h"
#include "string.h"
#include "kmalloc.h"
#include "assert.h"
#include "list.h"

inode_t *fs_root;

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
	if (len != 0 && path[len - 1] != '/') {
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

list_t *inode_cache;

inode_t *vfs_find_child(inode_t *parent, char *name)
{
	if (parent->type != FS_DIRECTORY) {
		return NULL;
	}
	if (parent->mount != NULL) {
		parent = parent->mount;
	}

	if (inode_cache == NULL) {
		inode_cache = list_create();
	}

	//Try to find it in the cache first
	foreach_list(inode_cache, curInode)
	{
		cachedNode_t *cachedInode = (void *)curInode->value;

		if (cachedInode->parent == parent &&
		    strncmp(name, cachedInode->name,
			    sizeof(cachedInode->name)) == 0) {
			return cachedInode->inode;
		}
	}

	struct inode *child = parent->i_op->find_child(parent, name);

	//Add it to the cache
	if (child != NULL) {
		cachedNode_t *newInodeToCache = kmalloc(sizeof(cachedNode_t));
		newInodeToCache->parent = parent;
		strncpy(newInodeToCache->name, name,
			sizeof(newInodeToCache->name));
		newInodeToCache->inode = child;
		list_enqueue(inode_cache, newInodeToCache);
	}

	return child;
}
int mount_root(inode_t *ino)
{
	assert(fs_root == NULL);
	assert(ino != NULL);

	fs_root = ino;

	return 0;
}
int mount(char *path, inode_t *ino)
{
	assert(ino != NULL);
	if (path[0] == '/' && path[1] == '\0') {
		return mount_root(ino);
	}

	inode_t *parent = vfs_namei(path);
	if (parent == NULL) {
		return -1;
	} else if (parent->mount != NULL) {
		return -1;
	}

	parent->mount = ino;
	return 0;
}
int umount(char *path)
{
	//Ex. path = /home/media/test
	char **tokens = tokenize(path);

	inode_t *cur = fs_root;

	int i = 0;
	while (tokens[i] != NULL) {
		char *child_name = tokens[i];

		cur = vfs_find_child(cur, child_name);
		if (cur == NULL) {
			return -1;
		}
		i++;
	}
	if (!cur->mount) {
		return -2;
	}
	cur->mount = NULL;

	kfree_arr(tokens);
	return 0;
}

file_t *vfs_open(const char *path)
{
	inode_t *inode = vfs_namei((char *)path);
	if (inode == NULL) {
		return NULL;
	}
	if (inode->mount != NULL) {
		inode = inode->mount;
	}

	file_t *file = kmalloc(sizeof(file_t));
	file->f_inode = inode;
	file->offset = 0;
	strncpy(file->path, path, sizeof(file->path));
	//file->path = cur;

	return file;
}
int vfs_close(file_t *file)
{
	//TODO proper cleanup, especially the inode
	kfree(file);

	return 0;
}
int vfs_read(file_t *file, void *buf, size_t offset, size_t size)
{
	if (file->f_inode->i_op->read) {
		return file->f_inode->i_op->read(file->f_inode, buf, offset,
						 size);
	} else {
		return -1;
	}
}
int vfs_write(file_t *file, void *buf, uint32_t offset, uint32_t size)
{
	if (file->f_inode && file->f_inode->i_op &&
	    file->f_inode->i_op->write) {
		return file->f_inode->i_op->write(file->f_inode, buf, offset,
						  size);
	} else {
		return -1;
	}
}
int vfs_mkdir(char *path, char *name)
{
	inode_t *parent = vfs_namei(path);
	if (parent == NULL) {
		return -1;
	}
	if (parent->mount != NULL) {
		parent = parent->mount;
	}
	if (parent->type != FS_DIRECTORY) {
		return -1;
	}
	if (parent->i_op && parent->i_op->mkdir) {
		struct dentry newDir;
		strncpy(newDir.name, name, 64);
		newDir.parent = parent;

		return parent->i_op->mkdir(NULL, &newDir);
	} else {
		return -1;
	}
}

inode_t *vfs_namei(char *path)
{
	if (path[0] == '/' && path[1] == '\0') {
		return fs_root;
	}

	char **tokens = tokenize(path);

	inode_t *cur = fs_root;
	if (tokens[0] == NULL) {
		cur = NULL;
	}

	int i = 0;
	while (tokens[i] != NULL) {
		char *child_name = tokens[i];

		cur = vfs_find_child(cur, child_name);
		if (cur == NULL) {
			return NULL;
		}

		i++;
	}
	kfree_arr(tokens);

	return cur;
}