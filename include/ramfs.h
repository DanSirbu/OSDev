#pragma once
#include "sys/types.h"
#include "fs.h"

void initramfs(size_t location);
inode_t *ramfs_getRoot();