#include "fs.h"
#include "vfs.h"
#include "string.h"
#include "kmalloc.h"
#include "assert.h"

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

vfs_node_t *vfs_find_child_in_tree(vfs_node_t *parent, char *name)
{
	for (vfs_node_t *cur = parent->children; cur != NULL; cur = cur->next) {
		if (strncmp(cur->name, name, FS_NAME_MAX_LEN) == 0) {
			return cur;
		}
	}

	return NULL;
}

void vfs_delete_node(vfs_node_t *node)
{
	if (node == NULL)
		return;

	if (node->children) {
		vfs_node_t *cur = node->children;

		while (cur != NULL) {
			vfs_node_t *next_child = cur->next;
			vfs_delete_node(cur);
			cur = next_child;
		}
	}
	//Actually free the node
	vfs_unlink_child(node->parent, node);
	node->parent = NULL;
	//TODO release inode
	node->inode = NULL;
	node->type = NULL;
	strcpy(node->name, "[deleted]");
	kfree(node);
}

void vfs_append_child(vfs_node_t *parent, vfs_node_t *child)
{
	assert(parent != NULL);
	assert(child != NULL);

	if (parent->children == NULL) {
		parent->children = child;
		child->parent = parent;
		return;
	}
	vfs_node_t *cur;
	for (cur = parent->children; cur->next != NULL; cur = cur->next)
		;

	cur->next = child;
	child->parent = parent;
}
void vfs_unlink_child(vfs_node_t *parent, vfs_node_t *child)
{
	assert(parent != NULL);
	assert(child != NULL);
	assert(parent->children != NULL);

	if (parent->children == child) {
		parent->children = NULL;
		return;
	}

	vfs_node_t *cur;
	for (cur = parent->children; cur->next != child; cur = cur->next) {
		cur->next = child->next;
	}
}

vfs_node_t *vfs_find_child(vfs_node_t *parent, char *name)
{
	vfs_node_t *found_child = vfs_find_child_in_tree(parent, name);
	if (found_child == NULL) { //Try to ask inode
		if (parent->inode->type == FS_DIRECTORY) {
			inode_t *child = parent->inode->i_op->find_child(
				parent->inode, name);
			if (child == NULL) {
				return NULL;
			}
			//Create child in tree
			vfs_node_t *node = kmalloc(sizeof(node));
			node->inode = child;
			strcpy(node->name, name);
			node->parent = parent;
			node->type = node->inode->type;

			vfs_append_child(parent, node);

			found_child = node;
		}
	}

	return found_child;
}
int mount_root(inode_t *ino)
{
	assert(fs_root == NULL);

	vfs_node_t *newNode = kcalloc(sizeof(vfs_node_t));
	strcpy(newNode->name, "/");
	newNode->parent = newNode;
	newNode->type = FS_MOUNTPOINT;
	newNode->inode = ino;

	fs_root = newNode;

	return 0;
}
int mount(char *path, inode_t *ino)
{
	if (path[0] == '/' && path[1] == '\0') {
		return mount_root(ino);
	}

	//Ex. path = /home/media/test
	char **tokens = tokenize(path);

	vfs_node_t *cur = fs_root;

	int i = 0;
	while (tokens[i + 1] != NULL) {
		char *child_name = tokens[i];

		cur = vfs_find_child(cur, child_name);
		if (cur == NULL) {
			return -2;
		}

		i++;
	}
	vfs_node_t *existingNode = vfs_find_child(cur, tokens[i]);
	if (existingNode != NULL) { //Already exists
		return -1;
	}

	vfs_node_t *newNode = kcalloc(sizeof(vfs_node_t));
	strcpy(newNode->name, tokens[i]);
	newNode->parent = cur;
	newNode->type = FS_MOUNTPOINT;
	newNode->inode = ino;

	vfs_append_child(cur, newNode);
	kfree_arr(tokens);

	return 0;
}
int umount(char *path)
{
	//Ex. path = /home/media/test
	char **tokens = tokenize(path);

	vfs_node_t *cur = fs_root;

	int i = 0;
	while (tokens[i] != NULL) {
		char *child_name = tokens[i];

		cur = vfs_find_child(cur, child_name);
		if (cur == NULL) {
			return -2;
		}

		i++;
	}

	//Delete children recursively and all this node
	vfs_delete_node(cur);

	return 0;
}

file_t *vfs_open(char *path)
{
	char **tokens = tokenize(path);

	vfs_node_t *cur = fs_root;

	int i = 0;
	while (tokens[i] != NULL) {
		char *child_name = tokens[i];

		cur = vfs_find_child(cur, child_name);
		if (cur == NULL) {
			return NULL;
		}

		i++;
	}

	file_t *file = kmalloc(sizeof(file_t));
	file->f_inode = cur->inode;
	//file->path = cur;

	return file;
}

int vfs_read(file_t *file, uint8_t *buf, size_t count, size_t *offset)
{
	if (file->f_inode->i_op->read) {
		return file->f_inode->i_op->read(file->f_inode, buf,
						 (uint32_t)offset, count);
	} else {
		return -1;
	}
}
inode_t *vfs_namei(char *path)
{
	char **tokens = tokenize(path);

	vfs_node_t *cur = fs_root;

	int i = 0;
	while (tokens[i] != NULL) {
		char *child_name = tokens[i];

		cur = vfs_find_child(cur, child_name);
		if (cur == NULL) {
			return NULL;
		}

		i++;
	}

	return cur->inode;
}