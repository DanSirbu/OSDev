#pragma once
#include "fs.h"

char **tokenize(char *path);
int mount(char *path, inode_t *ino);
int umount(char *path);
inode_t *vfs_namei(char *path);
void vfs_unlink_child(vfs_node_t *parent, vfs_node_t *child);