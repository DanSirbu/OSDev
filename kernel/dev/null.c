#include "fs.h"
#include "pipe.h"
#include "kmalloc.h"

inode_operations_t inode_pipe_noops = { .find_child = NULL,
					.get_child = NULL,
					.open = open_noop,
					.close = close_noop,
					.read = read_noop,
					.write = write_noop,
					.mkdir = NULL };

inode_t *make_null_pipe()
{
	inode_t *inode = kcalloc(sizeof(inode_t));
	inode->i_op = &inode_pipe_noops;
	inode->device = NULL;

	return inode;
}