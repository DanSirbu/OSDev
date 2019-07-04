#include "trap.h"
#include "task.h"
#include "syscall.h"
#include "fs.h"
#include "vfs.h"
#include "display.h"
#include "coraxstd.h"
#include "debug.h"
#include "time.h"
#include "pipe.h"

/*
 * Note: must be freed after done with it
 */
char **copy_arr(char *arr[])
{
	size_t numItems = array_length(arr);
	char **newArray = kmalloc((numItems + 1) * sizeof(char *));
	for (size_t x = 0; x < numItems; x++) {
		newArray[x] = strdup(arr[x]);
	}
	newArray[numItems] = NULL;

	return newArray;
}
int sys_execve(const char *filename, char *args1[], char *envs1[])
{
	char **args = { strdup(filename), NULL };
	//TODO MEMORY LEAK: how to free strdup(filename) after?
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
size_t sys_sbrk(uint32_t size)
{
	size = PG_ROUND_UP(size);
	size_t old_heap_addr = current->process->heap;

	mmap_flags_t flags = { .MAP_IMMEDIATELY = 1 };
	mmap(old_heap_addr, size, flags);
	current->process->heap = old_heap_addr + size;
	return old_heap_addr;
}
size_t sys_update_display(UNUSED uint32_t w, UNUSED uint32_t h,
			  uint32_t *buffered_data)
{
	display_update(buffered_data);
	return 0;
}
size_t sys_access(const char *path, UNUSED int amode)
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
file_t *getProcessFile(size_t fd)
{
	if (fd > MAX_FD_INDEX) {
		debug_print("FD %d does not exist\n", fd);
		return NULL;
	}
	return current->process->files[fd];
}
int sys_open(const char *path, int oflags)
{
	file_t *file = vfs_open(path);
	if (file == NULL) {
		set_userspace_errno(ENOENT);
		return -1;
	}

	int nextFD = getNextFD(current->process);
	if (nextFD == -1) {
		debug_print("Out of fd for process. Can't open %s\n", path);
		vfs_close(file);
		set_userspace_errno(ENFILE);
		return -1; //Out of file entries
	}
	current->process->files[nextFD] = file;
	//TODO, we should set flags on file_t not the inode
	file->f_inode->flags = oflags;

	return nextFD;
}
int sys_close(int fd)
{
	if (getProcessFile(fd) == NULL) {
		return -1;
	}
	file_t *file = getProcessFile(fd);
	vfs_close(file);
	current->process->files[fd] = NULL;

	return 0;
}

int sys_fstat(int fildes, struct stat *buf)
{
	file_t *file = getProcessFile(fildes);
	if (file == NULL) {
		set_userspace_errno(ENOMEM); //TODO, find better errno
		return -1;
	}

	memset(buf, 0, sizeof(struct stat));
	buf->st_size = file->f_inode->size;

	return 0;
}
uint32_t sys_read(int fd, void *buf, size_t n)
{
	file_t *file = getProcessFile(fd);
	if (file == NULL) {
		return -1;
	}
	int readAmount = vfs_read(file, buf, file->offset, n);
	if (readAmount > 0) {
		file->offset += readAmount;
	}

	return readAmount;
}
ssize_t sys_write(int fd, void *buf, size_t n)
{
	file_t *file = getProcessFile(fd);
	if (file == NULL) {
		return -1;
	}
	int writeAmount = vfs_write(file, buf, file->offset, n);
	if (writeAmount > 0) {
		file->offset += writeAmount;
	}

	return writeAmount;
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
			assert(-offset <
			       (ssize_t)file->offset); //TODO handle this case
		}
		file->offset += offset;
	} else {
		print(LOG_ERROR, "UNIMPLEMENTED: seek with whence %d\n",
		      whence);
		return -1;
	}

	return file->offset;
}
int sys_gettimeofday(struct timeval *p, void *z)
{
	return gettimeofday(p, z);
}

int sys_register_vars(struct userspace_vars *vars)
{
	assert(IS_IN_USERSPACE(vars));
	memcpy(&current->process->userspace_variables, vars,
	       sizeof(struct userspace_vars));

	return 0;
}
void set_userspace_errno(int errno)
{
	if (current->process->userspace_variables.errno_addr == NULL) {
		print(LOG_ERROR, "Userspace errno not registered in kernel\n");
		return;
	}
	*current->process->userspace_variables.errno_addr = errno;
}
int sys_setitimer(int which, const struct itimerval *value,
		  struct itimerval *ovalue)
{
	if (which == ITIMER_REAL) {
		if (ovalue != NULL) {
			memcpy(ovalue, &current->process->timer,
			       sizeof(struct itimerval));
		}
		memcpy((void *)&current->process->timer, (void *)value,
		       sizeof(struct itimerval));
	} else {
		print(LOG_ERROR, "setitimer: unsupported timer type: %d\n",
		      which);
		return -1;
	}
	return 0;
}
int sys_fcntl(int fd, int cmd, int flags)
{
	file_t *file = getProcessFile(fd);
	if (file == NULL) {
		return -1;
	}
	//TODO, handle cmd parameter

	//TODO, we should set flags on file_t not the inode
	//Also modify sys_open when the above TODO is done
	file->f_inode->flags = flags;

	return 0;
}
int sys_pipe(int fildes[2])
{
	int fd1 = getNextFD(current);
	int fd2 = getNextFD(current);
	if (fd1 == -1 || fd2 == -1) {
		return -1;
	}
	unix_pipe_t unix_pipe;
	make_unix_pipe(100, &unix_pipe);

	//TODO, vfs open

	current->process->files[fd1] = unix_pipe.read_pipe;
	current->process->files[fd1] = unix_pipe.write_pipe;

	return 0;
}
void syscall(int_regs_t *regs)
{
	if (getSyscallName(regs->eax) != NULL) {
		print(LOG_INFO, "Syscall %s\n", getSyscallName(regs->eax));
	} else {
		print(LOG_INFO, "Syscall 0x%x\n", regs->eax);
	}

	switch (regs->eax) {
		DEF_SYSCALL1(__NR_exit, exit, int, exitcode)
	case __NR_fork:
		regs->eax = sys_fork(regs);
		break;
		DEF_SYSCALL3(__NR_write, write, int, fd, char *, buf, int,
			     size);
		DEF_SYSCALL3(__NR_execve, execve, const char *, filename,
			     char **, args, char **, envs);
		DEF_SYSCALL3(__NR_clone, clone, void *, fn, void *, target_fn,
			     void *, child_stack);
		DEF_SYSCALL1(__NR_sbrk, sbrk, uint32_t, size);
		DEF_SYSCALL3(__NR_update_screen, update_display, size_t, w,
			     size_t, h, void *, buffered_data);
		DEF_SYSCALL2(__NR_access, access, const char *, path, int,
			     amode);

		DEF_SYSCALL3(__NR_fcntl, fcntl, int, fd, int, cmd, int, flags);
		DEF_SYSCALL2(__NR_fstat, fstat, int, fd, void *, stat);

		DEF_SYSCALL2(__NR_open, open, const char *, path, int, oflags);
		DEF_SYSCALL1(__NR_close, close, int, fd);
		DEF_SYSCALL3(__NR_read, read, int, fd, void *, buf, size_t, n);
		DEF_SYSCALL3(__NR_seek, seek, int, fd, long int, offset, int,
			     whence);
		DEF_SYSCALL2(__NR_time, gettimeofday, struct timeval *, p,
			     void *, z);
		DEF_SYSCALL1(__NR_signal_register, register_vars, void *, vars);
		DEF_SYSCALL2(__NR_kill, kill, pid_t, pid, int, sig);
		DEF_SYSCALL3(__NR_setitimer, setitimer, int, which,
			     const struct itimerval *, value,
			     struct itimerval *, ovalue);

		DEF_SYSCALL0(__NR_getpid, getPID);

		DEF_SYSCALL1(__NR_pipe, pipe, int, fildes[2]);
		DEF_SYSCALL3(__NR_waitpid, waitpid, pid_t, pid, int *, stat_loc,
			     int, options);
	default:
		print(LOG_ERROR, "Unhandled syscall 0x%x\n", regs->eax);
	}
}
void syscalls_install()
{
	register_isr_handler(SYSCALL_NO, syscall);
}