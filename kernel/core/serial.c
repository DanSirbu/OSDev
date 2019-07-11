// Taken from https://wiki.osdev.org/Serial_Ports
#include "serial.h"
#include "string.h"
#include "screen.h"
#include "config.h"
#include "pipe.h"
#include "vfs.h"
#include "coraxstd.h"

#define COM1 0x3f8
#define COM2 0x2F8

#define HEX_PREFIX "0x"

/* Serial "file" */
inode_t *serial_pipe;

/* Prototypes */
static void serial_print_string(char *message);

void initialize_serial_port(int port)
{
	outb(port + 1, 0x00); // Disable all interrupts
	outb(port + 3, 0x80); // Enable DLAB (set baud rate divisor)
	outb(port + 0, 0x01); // Set divisor to 1 (lo byte) 115.2k baud
	outb(port + 1, 0x00); //                  (hi byte)
	outb(port + 3, 0x03); // 8 bits, no parity, one stop bit
	outb(port + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
	outb(port + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

static int serial_write(UNUSED struct inode *node, void *buf,
			UNUSED uint32_t offset, uint32_t size)
{
	if (size == 0) {
		return 0;
	}
	char *strToPrint = kmalloc(size + 1);
	memcpy(strToPrint, buf, size);
	strToPrint[size] = '\0'; //Null terminate it

	serial_print_string(strToPrint);

	kfree(strToPrint);

	return size;
}

inode_operations_t inode_serial_ops = { .find_child = NULL,
					.get_child = NULL,
					.open = open_noop,
					.close = close_noop,
					.read = NULL,
					.write = serial_write,
					.mkdir = NULL };

void init_serial()
{
	initialize_serial_port(COM1);
	initialize_serial_port(COM2);
}
void init_serial_pipe()
{
	serial_pipe = calloc(sizeof(inode_t));
	serial_pipe->i_op = &inode_serial_ops;
}
/*int serial_received()
{
	return inb(PORT + 5) & 1;
}

char read_char_serial()
{
	while (serial_received() == 0)
		;

	return inb(PORT);
}*/

int is_transmit_empty(int port)
{
	return inb(port + 5) & 0x20;
}

void write_char_serial(char a)
{
#ifdef PRINT_TO_SCREEN
	if (a == '\n') {
		kprint_newline();
	} else {
		kprint_char(a);
	}
	return;
#endif
	while (is_transmit_empty(COM1) == 0)
		;

	outb(COM1, a);
}
static void serial_print_string(char *message)
{
	int i = 0;
	while (message[i] != '\0') {
		write_char_serial(message[i]);
		i++;
	}
}

void serial_print(char *message, va_list args)
{
	int i = 0;
	uint8_t minSize = 0;
	while (message[i] != '\0') {
		if (message[i] == '%') {
			i++;

			minSize = 0;
			if ((message[i] - ASCII_NUMBER_CONST) > 0 &&
			    (message[i] - ASCII_NUMBER_CONST) <
				    10) { // Its a number
				minSize = message[i] - ASCII_NUMBER_CONST;
				i++;
			}

			if (message[i] == '%') {
				write_char_serial('%');
			} else if (message[i] == 'x' || message[i] == 'p') {
				char buf[256];
				itoa(va_arg(args, size_t), buf, 16);

				uint8_t bufLen = strlen(buf);
				int padding_needed = minSize - bufLen;
				if (padding_needed > 0) {
					for (int x = bufLen - 1; x >= 0; x--) {
						buf[x + padding_needed] =
							buf[x];
					}
					for (int x = 0; x < padding_needed;
					     x++) {
						buf[x] = '0';
					}
					buf[bufLen + padding_needed] = '\0';
				}

				if (message[i] == 'p') {
					serial_print_string(HEX_PREFIX);
				}
				serial_print_string(buf);
			} else if (message[i] == 'd') {
				char buf[256];
				itoa(va_arg(args, uint32_t), buf, 10);
				serial_print_string(buf);
			} else if (message[i] == 's') {
				serial_print_string(va_arg(args, char *));
			}
		} else {
			write_char_serial(message[i]);
		}
		i++;
	}
}
