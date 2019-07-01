#pragma once
#include "fs.h"

char **tokenize(char *path);
int mount(char *path, inode_t *ino);
int umount(char *path);
inode_t *vfs_namei(char *path);
file_t *vfs_open(const char *path);
int vfs_read(file_t *file, void *buf, size_t offset, size_t size);
int vfs_write(file_t *file, void *buf, uint32_t offset, uint32_t size);

int vfs_close(file_t *file);
int vfs_mkdir(char *path, char *name);