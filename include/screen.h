#pragma once
#include "mmu.h"

/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT *COLUMNS_IN_LINE *LINES

void screen_init();
void kprint_newline(void);
void clear_screen(void);
void kprint(const char *str);
void kprint_char(char a);