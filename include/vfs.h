#pragma once
#include "fs.h"

char **tokenize(char *path);
int mount(char *path, inode_t *ino);
int umount(char *path);
inode_t *vfs_namei(char *path);