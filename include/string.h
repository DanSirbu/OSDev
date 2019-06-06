#pragma once
#include "types.h"

#define ASCII_NUMBER_CONST 0x30
#define ASCII_LETTER_CONST 0x57

size_t strlen(const char *str);
void reverse(char *str);
void itoa(uint32_t number, char *str, uint32_t base);
int strcmp(const char *str1, const char *str2);
char *strcpy(char *dst, char *src);
int strncmp(char *str1, char *str2, size_t max_len);

size_t array_length(char *arr[]);