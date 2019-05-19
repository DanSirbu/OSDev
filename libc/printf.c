#include "include/syscalls.h"
#include "include/types.h"
#include "include/string.h"

char print_buffer[2048];

#define COPY_BUF(buffer)                                                       \
	do {                                                                   \
		int write_size =                                               \
			MIN(n - out_string_index - 1, strlen(buffer));         \
		strncpy(&out_string[out_string_index], buffer, write_size);    \
		out_string_index += write_size;                                \
	} while (0)

void vprintf(char *message, va_list args);
void vsnprintf(char *s, size_t n, const char *message, va_list args);

int printf(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	//TODO proper return value
	return 0;
}
int snprintf(char *s, size_t n, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(s, n, format, args);
	va_end(args);

	//TODO proper return value
	return n;
}
void vsnprintf(char *s, size_t n, const char *message, va_list args)
{
	char *out_string = s;

	size_t format_index = 0;
	size_t out_string_index = 0;
	uint8_t minSize = 0;
	while (message[format_index] != '\0') {
		if (out_string_index == (n - 1)) {
			break;
		}
		if (message[format_index] == '%') {
			format_index++;

			//%%
			if (message[format_index] == '%') {
				out_string[out_string_index] = '%';
				out_string_index++;
				continue;
			}

			//ex: %5x
			minSize = 0;
			while (message[format_index] >= '0' &&
			       message[format_index] <= '9') {
				minSize = minSize * 10 +
					  (message[format_index] - '0');
				format_index++;
			}

			if (message[format_index] == 'x' ||
			    message[format_index] == 'p') {
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

				if (message[format_index] == 'p') {
					out_string[out_string_index] = '0';
					out_string_index++;
					out_string[out_string_index] = 'x';
					out_string_index++;
				}

				COPY_BUF(buf);
			} else if (message[format_index] == 'd') {
				char buf[256];
				itoa(va_arg(args, uint32_t), buf, 10);
				COPY_BUF(buf);
			} else if (message[format_index] == 's') {
				char *str = (va_arg(args, char *));

				COPY_BUF(str);
			}
		} else {
			out_string[out_string_index] = message[format_index];
			out_string_index++;
		}
		format_index++;
	}
	if (out_string_index > n - 1) {
		out_string_index = n - 1;
	}
	print_buffer[out_string_index] = '\0';
}
void vprintf(char *format, va_list args)
{
	vsnprintf(print_buffer, sizeof(print_buffer) / sizeof(print_buffer[0]),
		  format, args);
	write(1, print_buffer, strlen(print_buffer) + 1);
}
void assert_failed(char *statement, char *file, uint32_t line, const char *func)
{
	printf("ASSERT FAILED: %s at %s %d:%s\n", statement, file, line, func);
	while (1)
		;
}