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
char **tokenize(const char *path)
{
	char *newPath = (char *)path;
	if (path[0] == '/') {
		newPath = (char *)&path[1];
	}

	//Remove last '/' token if it is there
	char **retPath = split(newPath, "/");
	int retPathLength = array_length(retPath);
	if (retPathLength > 1) {
		if (strlen(retPath[retPathLength - 1]) ==
		    0) { //last token will be ""
			kfree(retPath[retPathLength - 1]);
			retPath[retPathLength - 1] = NULL;
		}
	}
	return retPath;
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
	if (child != NULL && !(child->flags & FS_FLAG_NOCACHE)) {
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
		} else if (cur->mount != NULL) {
			cur = cur->mount;
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

	return vfs_open_inode(inode, (char *)path);
}
file_t *vfs_open_inode(inode_t *inode, char *path)
{
	file_t *file = kmalloc(sizeof(file_t));
	file->f_inode = inode;
	file->offset = 0;
	strncpy(file->path, path, sizeof(file->path));
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
	if (file && file->f_inode && file->f_inode->i_op &&
	    file->f_inode->i_op->read) {
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
dir_dirent_t *vfs_get_child(file_t *file, uint32_t index)
{
	if (file->f_inode && file->f_inode->i_op->get_child) {
		return file->f_inode->i_op->get_child(file->f_inode, index);
	} else {
		return NULL;
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
		} else if (cur->mount != NULL) {
			cur = cur->mount;
		}

		i++;
	}
	kfree_arr(tokens);

	return cur;
}