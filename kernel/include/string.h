#pragma once
#include "coraxstd.h"

size_t strlen(const char *str);
void reverse(char *str);
void itoa(uint32_t number, char *str, uint32_t base);
int strcmp(const char *str1, const char *str2);
char *strcpy(char *dst, char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strconcat(char *dst, char *str1, char *str2);
int strncmp(char *str1, char *str2, size_t max_len);
char *strdup(const char *s);

size_t array_length(char *arr[]);

char *getBasename(const char *s);
char *getFilenameNoExt(char *s);

char *strsep(char **stringp, const char *delim);
char **split(char *str, char *delim);
char *strcat(char *dest, const char *src);

char *rindex(const char *s, int c);