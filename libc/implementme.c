#include "include/implementme.h"
#include "include/syscalls.h"
#include "include/assert.h"

int stdin = 0, stdout = 1, stderr = 2;

void sigaction(void)
{
	printf("DEBUG: sigaction\n");
}
extern char **environ;
char *getenv(const char *name)
{
	assert(environ != NULL);
	int i = 0;
	while (environ[i] != NULL) {
		if (strcmp(environ[i], name) == 0) {
			return environ[i + 1];
		}
		i += 2;
	}

	return NULL;
}

void raise(void)
{
}
int strcasecmp(const char *s1, const char *s2)
{
	return 0; //TODO
}
int strncasecmp(const char *s1, const char *s2, size_t n)
{
	return 0; //IMPLEMENT
}

FILE *fopen(const char *pathname, const char *mode)
{
	printf("Fread: %s TODO\n", pathname);
}
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	printf("Fread TODO\n");
}
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	printf("Fwrite: TODO\n");
}
int fseek(FILE *stream, long offset, int whence)
{
	printf("FSeek TODO\n");
}
int fclose(FILE *stream)
{
	printf("Fclose\n");
}
int fgetc(FILE *stream)
{
	printf("Fgetc: TODO\n");
}
char *fgets(char *s, int size, FILE *stream)
{
	printf("Fgets: %s\n", s);
}
int feof(FILE *stream)
{
	printf("feof \n");
}
int __isoc99_sscanf(const char *str, const char *format, ...)
{
	printf("sscanf: %s\n", str);
}
void *memchr(const void *s, int c, size_t n)
{
	printf("memchr \n");
	return s;
}
int access(const char *path, int amode)
{
	printf("Access: %s\n", path); //TODO implement
	return 1;
}
int __isoc99_fscanf(FILE *stream, const char *format, ...)
{
	printf("fscanf: \n");
}
int isatty(int fd)
{
	//TODO implement
	return 0; //1 if terminal or 0
}
void setbuf(FILE *stream, char *buf)
{
	printf("Setbuf\n");
}
int remove(const char *pathname)
{
	//unlink for files
	//rmdir for directories
	//todo
	return 0;
}
int mkdir(const char *path, mode_t mode)
{
	return 0; //TODO
}
ssize_t read(int fildes, void *buf, size_t nbyte)
{
	return 0; //TODO IMPORTANT
}
int ferror(FILE *stream)
{
	printf("ferror \n");
}
char *strerror(int errnum)
{
	printf("strerror\n");
}
long ftell(FILE *stream)
{
	printf("ftell\n");
}
off_t lseek(int fildes, off_t offset, int whence)
{
	printf("lseek\n");
}
int open(const char *path, int oflag, ...)
{
	//TODO
	return -1;
}
void __assert_fail(char *expr, char *file, int line, char *assert_fn)
{
	printf("Assert failed: %s, %d:%s\n", expr, line, file);
}
//__ctype_toupper_loc __xstat __errno_location
//	__assert_fail

int errno;

int *__errno_location()
{
	return &errno;
}
//From glibc
/* Get information about the file descriptor FD in BUF.  */
int __fxstat(int vers, int fd, struct stat *buf)
{
	return 0;
}
//From glibc
/* Get information about the file NAME in BUF.  */
int __xstat(int vers, const char *name, struct stat *buf)
{
	return -1;
}
/*void __stack_chk_fail_local()
{
	//noop
}*/
//strspn strcspn strpbrk