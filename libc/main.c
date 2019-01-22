#include "syscalls.h"
#include "types.h"
#include "main.h"
#include "string.h"

#define ASCII_NUMBER_CONST 0x30
#define ASCII_LETTER_CONST 0x57

#define HEX_PREFIX "0x"

char print_buffer[2048];
int ii;

void write_char_serial(char charVal);
void serial_print_string(char *str);
void vprintf(char *message, va_list args);

void printf(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
void vprintf(char *message, va_list args)
{
	ii = 0;

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

	print_buffer[ii] = '\0';
	write(1, print_buffer, ii);
}
void write_char_serial(char charVal)
{
	print_buffer[ii++] = charVal;
}
void serial_print_string(char *str)
{
	int i = 0;
	while (str[i] != '\0') {
		print_buffer[ii++] = str[i++];
	}
}