#pragma once
#include "mmu.h"

static char *vidptr = (char *)0x000b8000 + KERN_BASE;

/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT *COLUMNS_IN_LINE *LINES

void kprint_newline(void);
void clear_screen(void);
void kprint(const char *str);
