#include "fs.h"
#include "coraxstd.h"

extern size_t display_width, display_height;

size_t cursor_position = 0;
size_t font_width = 8;
size_t font_height = 8;

static int pipe_noop()
{
	return 0;
}
static int display_write(struct inode *node, void *buf, uint32_t offset,
			 uint32_t size)
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
					 .open = pipe_noop,
					 .close = pipe_noop,
					 .read = NULL,
					 .write = display_write,
					 .mkdir = NULL };

void initialize_terminal()
{
	display_pipe = kmalloc(sizeof(inode_t));
	display_pipe->i_op = &inode_display_ops;
}

void printStrToScreen(char *str)
{
	for (int x = 0; x < strlen(str); x++) {
		putchar(str[x]);
	}
}

void putchar(char c)
{
	drawCharacter(c, cursor_position % display_width,
		      cursor_position / display_height);

	cursor_position += font_width;
	if (cursor_position % display_width == 0) {
		cursor_position += font_height * display_width;
	}
	if (cursor_position > display_width * display_height) {
		cursor_position = 0;
	}
}