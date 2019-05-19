#pragma once
#include "types.h"

uint32_t strlen(char *str);
void reverse(char *str);
void itoa(uint32_t number, char *str, uint32_t base);
int strcmp(char *str1, char *str2);
void strcpy(char *dst, char *src);
void strcpy_max_len(char *src, char *dest, uint32_t maxLen);
char *strdup(const char *s);
int atoi(const char *nptr);
char *strncpy(char *dest, const char *src, size_t n);