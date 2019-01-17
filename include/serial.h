#pragma once
#include "io.h"
#include "string.h"
#include <stdarg.h>

void init_serial();
void serial_print(char *message, va_list args);

#define fail(reason) assert(0 && (reason))

#define assert(expr)                                                      \
	(expr) ? (void)0 : assert_failed(#expr, __FILE__, __LINE__, __FUNCTION__)

extern void assert_failed(char *statement, char *file, uint32_t line, const char *func);

void debug_print(char *message, ...);
void abort();