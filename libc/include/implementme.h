#pragma once
#include "sys/types.h"

void sigaction(void);
char *getenv(const char *name);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);

FILE *fopen(const char *pathname, const char *mode);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
int fclose(FILE *stream);
char *fgets(char *s, int size, FILE *stream);
int feof(FILE *stream);
int getc(FILE *stream);
int ungetc(int c, FILE *stream);
int __isoc99_sscanf(const char *str, const char *format, ...);
char *strtok(char *str, const char *delim);
void *memchr(const void *s, int c, size_t n);
char *strchr(const char *s, int c);
int access(const char *path, int amode);