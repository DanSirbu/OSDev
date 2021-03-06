#include "fs.h"
#include "coraxstd.h"
#include "kmalloc.h"
#include "string.h"
#include "display.h"
#include "terminal.h"
#include "pipe.h"

size_t cursor_position = 0;

size_t terminal_width = 80;

/* Prototypes */
void putchar(char c);

static int display_write(UNUSED struct inode *node, void *buf,
			 UNUSED uint32_t offset, uint32_t size)
{
	if (size == 0) {
		return 0;
	}
	char *strToPrint = kmalloc(size + 1);
	memcpy(strToPrint, buf, size);
	strToPrint[size] = '\0'; //Null terminate it

	printStrToScreen(strToPrint);

	kfree(strToPrint);

	return size;
}

//Takes input and writes it to screen
inode_t *display_pipe;

inode_operations_t inode_display_ops = { .find_child = NULL,
					 .get_child = NULL,
					 .open = open_noop,
					 .close = close_noop,
					 .read = NULL,
					 .write = display_write,
					 .mkdir = NULL };

void initialize_terminal()
{
	display_pipe = kcalloc(sizeof(inode_t));
	display_pipe->i_op = &inode_display_ops;
}

void printStrToScreen(char *str)
{
	if (strncmp(str, "\033[H\033[2J", sizeof("\033[H\033[2J")) == 0) {
		display_clear();
		cursor_position = 0;
	} else {
		for (uint32_t x = 0; x < strlen(str); x++) {
			putchar(str[x]);
		}
	}
}

int prevAnsii = false;
void putchar(char c)
{
	if (c == '\n') {
		drawCharacter(' ', cursor_position % terminal_width,
			      cursor_position / terminal_width);
		cursor_position = cursor_position -
				  (cursor_position % terminal_width) +
				  terminal_width;
		update_cursor();
		return;
	} else if (c == '\b') {
		if (cursor_position == 0) {
			return;
		}
		drawCharacter(' ', cursor_position % terminal_width,
			      cursor_position / terminal_width);
		cursor_position -= 1;
		update_cursor();
		return;
	} else if (c == '\033') { //TODO, actually use ansii escape codes
		prevAnsii = true;
	}
	if (!prevAnsii) {
		drawCharacter(c, cursor_position % terminal_width,
			      cursor_position / terminal_width);

		cursor_position += 1;

		update_cursor();
	} else if (c == 'm') {
		prevAnsii = false;
	}
}

void update_cursor()
{
	drawCharacter('\xdb', cursor_position % terminal_width,
		      cursor_position / terminal_width);
}