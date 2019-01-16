#pragma once
#include "io.h"
#include "string.h"
#include <stdarg.h>

#define HEX_PREFIX "0x"
void init_serial();
void kpanic_fmt(char *message, ...);
void kpanic_fmt1(char *message, va_list args);

#define fail(reason) assert(0 && (reason))

#define assert(expr)                                                      \
	(expr) ? (void)0 : assert_failed(#expr, __FILE__, __LINE__, __FUNCTION__)

extern void assert_failed(char *statement, char *file, uint32_t line, const char *func);

void debug_print(char *message, ...);