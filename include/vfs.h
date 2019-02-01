#pragma once

char **tokenize(char *path);
int mount(char *path, inode_t *ino);
int umount(char *path);