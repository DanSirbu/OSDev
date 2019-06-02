#include "trap.h"
#include "task.h"
#include "syscalls.h"
#include "fs.h"
#include "vfs.h"
#include "display.h"
#include "coraxstd.h"

int sys_write(int fd, char *buf, int size)
{
	print(LOG_INFO, "write->fd(%d) %s", fd, buf);
	return 1;
}
/*
 * Note: must be freed after done with it
 */
char **copy_arr(char *arr[])
{
	size_t numItems = array_length(arr);
	char **newArray = kmalloc((numItems + 1) * sizeof(char *));
	for (size_t x = 0; x < numItems; x++) {
		size_t strLen = strlen(arr[x]) + 1; //Include null terminator
		newArray[x] = kmalloc(strLen);
		memcpy(newArray[x], arr[x], strLen);
	}
	newArray[numItems] = NULL;

	return newArray;
	}
int sys_execve(const char *filename, char *args1[], char *envs1[])
{
	char **args = { filename, NULL };
	char **envs = { NULL };

	if (args1 != NULL) {
		//TODO MEMORY LEAK: how to free this after easily?
		args = copy_arr(args1);
	}
	if (envs1 != NULL) {
		envs = copy_arr(envs1);
	}
	return execve((const char *)filename, args, envs);
}
extern task_t *current;
vptr_t sys_sbrk(uint32_t size)
{
	size = PG_ROUND_UP(size);
	vptr_t old_heap_addr = current->process->heap;

	mmap_flags_t flags = { .MAP_IMMEDIATELY = 1,
			       .IGNORE_PAGE_MAPPED = 0,
			       .IGNORE_FRAME_REUSE = 0 };
	mmap(old_heap_addr, size, flags);
	current->process->heap = old_heap_addr + size;
	return old_heap_addr;
}
vptr_t sys_update_display(uint32_t w, uint32_t h, uint32_t *buffered_data)
{
	int z = w + h;
	display_update(buffered_data);
	z++;
	return 0;
}
size_t sys_access(const char *path, int amode)
{
	print(LOG_INFO, "Access: %s\n", path);
	//TODO use amode
	file_t *file = vfs_open(path);
	if (file == NULL) {
		return -1;
	}
	vfs_close(file);
	return 0;
}
size_t sys_open(const char *path)
{
	file_t *file = vfs_open(path);
	if (file == NULL) {
		return -1;
	}

	if (current->process->lastFileIndex == 10) {
		debug_print("Out of fd for process. Can't open %s\n", path);
		kfree(file);
		return NULL; //Out of file entries
	}

	size_t fd = current->process->lastFileIndex;
	current->process->files[fd] = file;
	current->process->lastFileIndex++;

	return fd;
}
file_t *getProcessFile(size_t fd)
{
	if (fd > current->process->lastFileIndex) {
		debug_print("FD %d does not exist\n", fd);
		return NULL;
	}
	return current->process->files[fd];
}
uint32_t sys_read(int fd, void *buf, size_t n)
{
	file_t *file = getProcessFile(fd);
	if (file == NULL) {
		return -1;
	}
	int readAmount = vfs_read(file, buf, n, file->offset);
	if (readAmount > 0) {
		file->offset += readAmount;
	}

	return readAmount;
}
int sys_seek(int fd, long int offset, int whence)
{
	file_t *file = getProcessFile(fd);
	if (file == NULL) {
		return -1;
	}

	if (whence == SEEK_SET) {
		assert(offset >= 0); //TODO handle < 0
		file->offset = offset;
	} else if (whence == SEEK_CUR) {
		if (offset < 0) {
			assert(-offset < file->offset); //TODO handle this case
		}
		file->offset += offset;
	} else {
		print(LOG_ERROR, "UNIMPLEMENTED: seek with whence %d\n",
		      whence);
		return -1;
	}

	return file->offset;
}
void syscall(int_regs_t *regs)
{
	print(LOG_INFO, "Syscall 0x%x\n", regs->eax);

	switch (regs->eax) {
		DEF_SYSCALL1(__NR_exit, exit, int, exitcode)
	case __NR_fork:
		regs->eax = sys_fork(regs);
		break;
		DEF_SYSCALL3(__NR_write, write, int, fd, char *, buf, int,
			     size);
		DEF_SYSCALL3(__NR_execve, execve, const char *, filename,
			     char **, args, char **, envs);
	case __NR_clone:
		sys_clone(regs);
		break;
		DEF_SYSCALL1(__NR_sbrk, sbrk, uint32_t, size);
		DEF_SYSCALL3(10, update_display, size_t, w, size_t, h,
			     uint32_t *, buffered_data);
		DEF_SYSCALL2(__NR_access, access, const char *, path, int,
			     amode);

		DEF_SYSCALL1(__NR_open, open, const char *, path);
		DEF_SYSCALL3(__NR_read, read, int, fd, void *, buf, size_t, n);
		DEF_SYSCALL3(__NR_seek, seek, int, fd, long int, offset, int,
			     whence);
	default:
		print(LOG_ERROR, "Unhandled syscall 0x%x\n", regs->eax);
	}
}
void syscalls_install()
{
	register_isr_handler(SYSCALL_NO, syscall);
}