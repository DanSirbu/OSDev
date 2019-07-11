#include "coraxstd.h"
#include "circularqueue.h"
#include "kmalloc.h"
#include "assert.h"
#include "pipe.h"
#include "task.h"

/* Prototypes */
static int pipe_read(struct inode *node, void *buf, uint32_t offset,
		     uint32_t size);
static int pipe_write(struct inode *inode, void *buf, uint32_t offset,
		      uint32_t size);

inode_operations_t inode_pipe_ops = { .find_child = NULL,
				      .get_child = NULL,
				      .open = open_noop,
				      .close = close_noop,
				      .read = pipe_read,
				      .write = pipe_write,
				      .mkdir = NULL };

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

inode_t *make_pipe(size_t size)
{
	CircularQueue *queue = CircularQueueCreate(size);

	inode_t *inode = kcalloc(sizeof(inode_t));
	inode->i_op = &inode_pipe_ops;
	inode->device = queue;

	return inode;
}
void make_unix_pipe(size_t size, unix_pipe_t *unixpipe)
{
	inode_t *pipe = make_pipe(size);
	unixpipe->read_pipe = pipe;
	unixpipe->write_pipe = pipe;
}

static int pipe_read(struct inode *node, void *buf, uint32_t offset,
		     uint32_t size)
{
	assert(node != NULL);
	assert(node->device != NULL);

	CircularQueue *queue = node->device;
	uint8_t *buffer = buf;

	int readAmount = 0;
	while (readAmount < size) {
		int ret = CircularQueueFront(queue);
		if (ret == (int)NULL) { //TODO, what if we enqueue 0?
			if (node->flags & O_NONBLOCK) {
				return readAmount;
			} else {
				wakeup_queue(queue->write_queue);
				sleep_on(queue->read_queue);
				continue;
			}
		}
		buffer[readAmount] = ret;
		assert(CircularQueueDeQueue(queue) == true);
		readAmount++;
	}
	return readAmount;
}
static int pipe_write(struct inode *node, void *buf, uint32_t offset,
		      uint32_t size)
{
	assert(node->device != NULL);
	CircularQueue *queue = node->device;
	uint8_t *buffer = buf;

	int writeAmount = 0;
	while (writeAmount < size) {
		bool ret = CircularQueueEnQueue(queue, buffer[writeAmount]);
		if (ret == false) { //TODO, what if we enqueue 0?
			if (node->flags & O_NONBLOCK) {
				return writeAmount;
			} else {
				wakeup_queue(queue->read_queue);
				sleep_on(queue->write_queue);
				continue;
			}
		}
		if (!(node->flags & O_NONBLOCK)) {
			wakeup_queue(queue->read_queue);
		}
		writeAmount++;
	}
	return writeAmount;
}
int pipe_noop()
{
	return 0;
}
int open_noop(UNUSED struct inode inode, UNUSED uint32_t options)
{
	return 0;
}
int close_noop(UNUSED struct inode inode)
{
	return 0;
}
int read_noop(struct inode *inode, void *buf, uint32_t offset, uint32_t size)
{
	return size;
}
int write_noop(struct inode *inode, void *buf, uint32_t offset, uint32_t size)
{
	return size;
}