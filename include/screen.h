#pragma once

/* video memory begins at address 0xb8000 */
static char *vidptr = (char *)0xc00b8000;

/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT *COLUMNS_IN_LINE *LINES

void kprint_newline(void);
void clear_screen(void);
void kprint(const char *str);
