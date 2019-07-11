#pragma once
#include "fs.h"

char **tokenize(char *path);
int mount(char *path, inode_t *ino);
int umount(char *path);
inode_t *vfs_namei(char *path);
file_t *vfs_open(const char *path);
file_t *vfs_open_inode(inode_t *inode, char *path);
int vfs_read(file_t *file, void *buf, size_t offset, size_t size);
int vfs_write(file_t *file, void *buf, uint32_t offset, uint32_t size);

dir_dirent_t *vfs_get_child(file_t *file, uint32_t index);

int vfs_close(file_t *file);
int vfs_mkdir(char *path, char *name);