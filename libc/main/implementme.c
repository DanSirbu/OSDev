#include "implementme.h"
#include "syscalls.h"
#include "assert.h"
#include "coraxstd.h"

FILE stdin_f = { .fd = STDIN_FILENO };
FILE stdout_f = { .fd = STDOUT_FILENO };
FILE stderr_f = { .fd = STDERR_FILENO };

FILE *stdin = &stdin_f;
FILE *stdout = &stdout_f;
FILE *stderr = &stderr_f;

extern char **environ; //in crt0.s
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
int strcasecmp(const char *s1, const char *s2)
{
	char *p1 = s1;
	char *p2 = s2;
	while (tolower(*p1) == tolower(*p2)) {
		if (*p1 == '\0') {
			return 0;
		}
		p1++;
		p2++;
	}
	return tolower(*p1) - tolower(*p2);
}
int strncasecmp(const char *s1, const char *s2, size_t n)
{
	size_t i;
	for (i = 0; i < n; i++) {
		if (tolower(s1[i]) != tolower(s2[i]) ||
		    (s1[i] == '\0' && s2[i] == '\0')) {
			return tolower(s1[i]) != tolower(s2[i]);
		}
	}
	return 0;
}

FILE *fopen(const char *pathname, const char *mode)
{
	//TODO, handle mode
	int fd = open(pathname, 0);
	if (fd < 0) {
		return NULL;
	}

	FILE *file = (FILE *)malloc(sizeof(FILE));
	file->fd = fd;

	return file;
}
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	assert(stream != NULL);
	size_t numElementsRead =
		(uint32_t)read(stream->fd, ptr, nmemb * size) / size;
	return numElementsRead;
}
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	assert(stream != NULL);
	return write(stream->fd, ptr, size * nmemb) / size;
}
int fseek(FILE *stream, long offset, int whence)
{
	assert(stream != NULL);
	long int ret = call_seek(stream->fd, offset, whence);
	if (ret == -1) {
		return -1;
	}
	return 0;
}
int fclose(FILE *stream)
{
	assert(stream != NULL);

	int ret = sys_close(stream->fd);
	if (ret != 0) {
		return ret;
	}
	free(stream);
	return 0;
}
int fgetc(FILE *stream)
{
	uint8_t buf;
	int ret = fread(
		&buf, 1, 1,
		stream); //TODO, this is highly inefficient, create a buffer in FILE

	if (ret <= 0) {
		return EOF;
	}
	return buf;
}
int getc(FILE *stream)
{
	return fgetc(stream);
}
int ungetc(int c, FILE *stream)
{
	fprintf(stderr, "IMPLEMENT: ungetc\n");
	return -1;
}
char *fgets(char *s, int size, FILE *stream)
{
	assert(s != NULL);

	if (size <= 1) { //Can't read negative
		if (size - 1 != 0)
			return 0;
		*s = 0;
		return s;
	}
	int ret = read(stream->fd, s, size);
	if (ret <= 0) {
		return NULL;
	}
	for (int x = 0; x < size; x++) {
		if (s[x] == '\n') {
			fseek(stream, (x + 1) - size, SEEK_CUR);
			s[x + 1] = '\0';
			break;
		}
	}

	return s;
}
int feof(FILE *stream)
{
	fprintf(stderr, "IMPLEMENT: feof\n");
}
void *memchr(const void *s, int c, size_t n)
{
	uint8_t *cur = s;
	for (size_t i = 0; i < n; i++) {
		if (cur[i] == (uint8_t)c) {
			return &cur[i];
		}
	}
	return NULL;
}
int isatty(int fd)
{
	fprintf(stderr, "IMPLEMENT: isatty\n");
	return 0; //1 if terminal or 0
}
void setbuf(FILE *stream, char *buf)
{
	fprintf(stderr, "IMPLEMENT: setbuf\n");
}
int remove(const char *pathname)
{
	fprintf(stderr, "IMPLEMENT: remove\n");
	//unlink for files
	//rmdir for directories
	//todo
	return 0;
}
int mkdir(const char *path, mode_t mode)
{
	fprintf(stderr, "IMPLEMENT: mkdir\n");
	return 0; //TODO
}
int ferror(FILE *stream)
{
	fprintf(stderr, "IMPLEMENT: isatty\n");
	return 0;
}
char *strerror(int errnum)
{
	fprintf(stderr, "IMPLEMENT: strerror\n");
	return "";
}
long ftell(FILE *stream)
{
	fprintf(stderr, "IMPLEMENT: ftell\n");
	return 0;
}
off_t lseek(int fildes, off_t offset, int whence)
{
	return call_seek(fildes, offset, whence);
}
int open(const char *path, int oflag, ...)
{
	//TODO handle oflag and mode
	return sys_open(path, oflag);
}
void __assert_fail(char *expr, char *file, int line, char *assert_fn)
{
	printf("Assert failed: %s, %d:%s\n", expr, line, file);
}

int errno;

int *__errno_location()
{
	return &errno;
}
//From glibc
/* Get information about the file descriptor FD in BUF.  */
int __fxstat(int vers, int fd, struct stat *buf)
{
	fprintf(stderr, "IMPLEMENT: fxstat\n");
	return 0;
}
//From glibc
/* Get information about the file NAME in BUF.  */
int __xstat(int vers, const char *name, struct stat *buf)
{
	fprintf(stderr, "IMPLEMENT: xstat\n");
	return -1;
}
int stat(const char *restrict path, struct stat *restrict buf)
{
	fprintf(stderr, "IMPLEMENT: stat\n");
	return -1;
}
typedef unsigned long sigset_t;
int sigemptyset(sigset_t *set)
{
	fprintf(stderr, "IMPLEMENT: sigemptyset\n");
	return 0;
}
typedef struct {
	unsigned long fds_bits[1024 / 8 / sizeof(unsigned long)];
} fd_set;
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds,
	   struct timeval *timeout)
{
	fprintf(stderr, "IMPLEMENT: select\n");
	return 0;
}
weak_alias(sqrtf, __sqrt_finite);