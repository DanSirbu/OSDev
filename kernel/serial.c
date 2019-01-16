// Taken from https://wiki.osdev.org/Serial_Ports
#include "serial.h"
#include "string.h"

#define PORT 0x3f8 /* COM1 */

void init_serial()
{
	outb(PORT + 1, 0x00); // Disable all interrupts
	outb(PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
	outb(PORT + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
	outb(PORT + 1, 0x00); //                  (hi byte)
	outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
	outb(PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
	outb(PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

int serial_received()
{
	return inb(PORT + 5) & 1;
}

char read_char_serial()
{
	while (serial_received() == 0)
		;

	return inb(PORT);
}

int is_transmit_empty()
{
	return inb(PORT + 5) & 0x20;
}

void write_char_serial(char a)
{
	while (is_transmit_empty() == 0)
		;

	outb(PORT, a);
}
static void serial_print_string(char *message)
{
	int i = 0;
	while (message[i] != '\0') {
		write_char_serial(message[i]);
		i++;
	}
}

void kpanic_fmt(char *message, ...)
{
	va_list args;
	va_start(args, message);
	kpanic_fmt1(message, args);
	va_end(args);
}
void kpanic_fmt1(char *message, va_list args)
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