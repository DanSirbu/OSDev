#pragma once
#include "fs.h"

inode_t *make_null_pipe();
inode_t *make_pipe(size_t size);