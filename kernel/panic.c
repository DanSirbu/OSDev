#include "serial.h"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

void debug_print(char *message, ...) {
    va_list args;
	va_start(args, message);
    kpanic_fmt1(message, args);
    va_end(args);
}

void abort() {
    debug_print("Aborting! 0x%x\n", 0x123, 1);
    cli();
    asm("hlt");
    while(1);
}

void assert_failed(char *statement, char *file, uint32_t line, const char *func)
{
	kpanic_fmt(ANSI_COLOR_RED "%s:%d: error in %s: %s" ANSI_COLOR_RESET "\n", file, line, func, statement);
    abort();
}