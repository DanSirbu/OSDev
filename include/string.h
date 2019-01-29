#pragma once
#include "types.h"

#define ASCII_NUMBER_CONST 0x30
#define ASCII_LETTER_CONST 0x57

uint32_t strlen(char *str);
void reverse(char *str);
void itoa(u32 number, char *str, u32 base);
int strcmp(char *str1, char *str2);
void strcpy(char *dst, char *src);
int strncmp(char *str1, char *str2, size_t max_len);
void strcpy_max_len(char *src, char *dest, uint32_t maxLen);