#pragma once
#include "sys/types.h"

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);

void reverse(const char *str);
void itoa(uint32_t number, char *str, uint32_t base);
int strcmp(const char *str1, const char *str2);
char *strcpy(char *dst, char *src);
char *strdup(const char *s);
int atoi(const char *nptr);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
int strncmp(const char *s1, const char *s2, size_t n);
char *strsep(char **stringp, const char *delim);
char **tokenize(char *str, char *delim);

int fprintf(FILE *stream, const char *format, ...);

int puts(const char *s);
int fputc(int c, FILE *stream);
int sprintf(char *s, const char *format, ...);
int vsprintf(char *str, const char *format, va_list ap);