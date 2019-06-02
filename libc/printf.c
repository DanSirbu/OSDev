#include "include/syscalls.h"
#include "include/types.h"
#include "include/string.h"
#include "include/assert.h"

char print_buffer[2048];

#define COPY_BUF(buffer)                                                       \
	do {                                                                   \
		int write_size =                                               \
			MIN(n - out_string_index - 1, strlen(buffer));         \
		if (!onlyReturnSize) {                                         \
			strncpy(&out_string[out_string_index], buffer,         \
				write_size);                                   \
		}                                                              \
		out_string_index += write_size;                                \
	} while (0)

int vprintf(const char *message, va_list args);
int vsnprintf(char *s, size_t n, const char *message, va_list args);

int fprintf(FILE *stream, const char *format, ...)
{
	assert(stream != NULL);
	//TODO handle other than stdout
	assert_msg(stream->fd == 1 || stream->fd == 2, "FD: %d\n", stream->fd);

	va_list args;
	va_start(args, format);
	int ret = vprintf(format, args);
	va_end(args);
	return ret;
}
int puts(const char *s)
{
	return printf("%s", s);
}
int printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int ret = vprintf(fmt, args);
	va_end(args);

	return ret;
}

int sprintf(char *s, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = vsnprintf(s, SIZE_MAX, format,
			    args); //TODO maybe? unbounded sprintf
	va_end(args);

	return ret;
}
int snprintf(char *s, size_t n, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = vsnprintf(s, n, format, args);
	va_end(args);

	return ret;
}
int vsprintf(char *str, const char *format, va_list ap)
{
	return vsnprintf(str, SIZE_MAX, format,
			 ap); //TODO maybe make infinite print
}
int vsnprintf(char *s, size_t n, const char *message, va_list args)
{
	bool onlyReturnSize = false;
	if (s == NULL || n == 0) {
		onlyReturnSize = true;
	}
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
			if (message[format_index] ==
			    '.') { //TODO, handle dot, ignore for now
				format_index++;
			}

			//%%
			if (message[format_index] == '%') {
				if (!onlyReturnSize) {
					out_string[out_string_index] = '%';
				}
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
					if (!onlyReturnSize) {
						out_string[out_string_index] =
							'0';
					}
					out_string_index++;
					if (!onlyReturnSize) {
						out_string[out_string_index] =
							'x';
					}
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
			if (!onlyReturnSize) {
				out_string[out_string_index] =
					message[format_index];
			}
			out_string_index++;
		}
		format_index++;
	}
	if (out_string_index > n - 1) {
		out_string_index = n - 1;
	}
	if (!onlyReturnSize) {
		out_string[out_string_index] = '\0';
	}
	return out_string_index;
}
int vprintf(const char *format, va_list args)
{
	int ret = vsnprintf(print_buffer,
			    sizeof(print_buffer) / sizeof(print_buffer[0]),
			    format, args);
	write(1, print_buffer, strlen(print_buffer) + 1);

	return ret;
}
void assert_failed(char *statement, char *file, uint32_t line, const char *func,
		   char *format, ...)
{
	printf("ASSERT FAILED: %s at %s %d:%s   - ", statement, file, line,
	       func);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\n");
	while (1)
		;
}
int fputc(int c, FILE *stream)
{
	return write(stream->fd, &c, 1);
}