#pragma once
#include "types.h"

#define ASCII_NUMBER_CONST 0x30
#define ASCII_LETTER_CONST 0x57

u32 strlen(char *str);
void reverse(char *str);
void itoa(u32 number, char *str, u32 base);
void strcpy_max_len(char *src, char *dest, uint32_t maxLen);