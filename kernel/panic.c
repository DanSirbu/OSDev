#include "serial.h"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

char *log_levels[4] = { "ALL:     ", "INFO:    ", "WARNING: ", "ERROR:   " };

void print(uint8_t level, char *message, ...)
{
	if (level < MIN_LOG_LEVEL)
		return;
	va_list args;
	va_start(args, message);
	serial_print(log_levels[level], NULL);
	serial_print(message, args);
	va_end(args);
}

void debug_print(char *message, ...)
{
	va_list args;
	va_start(args, message);
	serial_print(message, args);
	va_end(args);
}

void halt()
{
	cli();
	asm("hlt");
	while (1)
		;
}

void assert_failed(char *statement, char *file, uint32_t line, const char *func)
{
	debug_print(ANSI_COLOR_RED "%s:%d: error in %s: %s" ANSI_COLOR_RESET
				   "\n",
		    file, line, func, statement);
	halt();
}