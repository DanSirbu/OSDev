#include "fs.h"
#include "pipe.h"
#include "kmalloc.h"
#include "task.h"
#include "proc.h"

struct inode *proc_find_child(struct inode *parent, char *name);
static dir_dirent_t *proc_get_child(struct inode *parent, uint32_t index);
static int proc_read(struct inode *inode, void *buf, uint32_t offset,
		     uint32_t size);

inode_operations_t inode_proc_ops = { .find_child = proc_find_child,
				      .get_child = proc_get_child,
				      .open = open_noop,
				      .close = close_noop,
				      .read = proc_read,
				      .write = write_noop,
				      .mkdir = NULL };

static inode_t *proc_root;
static dir_header_t *proc_root_dirheader;
inode_t *make_proc_pipe()
{
	inode_t *inode = kcalloc(sizeof(inode_t));
	inode->i_op = &inode_proc_ops;
	inode->device = NULL;
	inode->type = FS_DIRECTORY;

	proc_root = inode;
	proc_root_dirheader = kmalloc(sizeof(proc_root_dirheader));

	return inode;
}

extern list_t *task_list;

struct inode *proc_find_child(struct inode *parent, char *name)
{
	if (parent != proc_root) {
		return NULL;
	}

	foreach_list(task_list, nodeVal)
	{
		task_t *curTask = (task_t *)nodeVal->value;
		if (strncmp(curTask->name, name, 255) == 0) {
			inode_t *newInode = malloc(sizeof(inode_t));
			newInode->ino = curTask->id;
			newInode->i_op = &inode_proc_ops;
			newInode->type = FS_FILE;

			return newInode;
		}
	}
}
static dir_dirent_t *proc_get_child(struct inode *parent, uint32_t index)
{
	if (proc_root == parent) {
		uint32_t i = 0;
		foreach_list(task_list, nodeVal)
		{
			task_t *curTask = (task_t *)nodeVal->value;

			if (i == index) {
				dir_dirent_t *dirent =
					malloc(sizeof(dir_dirent_t));
				dirent->ino = curTask->id;
				strncpy(dirent->name, curTask->name,
					sizeof(dirent->name));
				return dirent;
			}
			i++;
		}
	}

	return NULL;
}
static int proc_read(struct inode *inode, void *buf, uint32_t offset,
		     uint32_t size)
{
	if (proc_root == inode) {
		proc_root_dirheader->num_dirs = getNumTasksInTasklist();

		char *srcBuf = (char *)proc_root_dirheader;
		int rSize;
		for (rSize = 0; rSize < MIN(size, sizeof(proc_root_dirheader));
		     rSize++) {
			((char *)buf)[rSize] = srcBuf[rSize];
		}

		return rSize - 1;
	}
	task_t *task = getTask(inode->ino);
	char procInfo[255];
	itoa(task->id, procInfo, 10);

	strconcat(procInfo, procInfo, ":");
	strconcat(procInfo, procInfo, task->name);

	int rSize;
	for (rSize = 0; rSize < MIN(size, strlen(procInfo) + 1); rSize++) {
		((char *)buf)[rSize] = procInfo[rSize];
	}
	if (rSize > 0) {
		rSize -= 1;
	}

	return rSize;
}