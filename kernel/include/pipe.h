#pragma once
#include "fs.h"

typedef struct {
	inode_t *read_pipe;
	inode_t *write_pipe;
} unix_pipe_t;

inode_t *make_null_pipe();
inode_t *make_pipe(size_t size);
void make_unix_pipe(size_t size, unix_pipe_t *unixpipe);
int pipe_noop(void);