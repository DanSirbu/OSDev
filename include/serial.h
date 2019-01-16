#pragma once
#include "io.h"
#include "string.h"
#include <stdarg.h>

#define HEX_PREFIX "0x"
void init_serial();
void kpanic_fmt(char *message, ...);
void kpanic_fmt1(char *message, va_list args);
void kpanic(char *message);

#define fail(reason) assert(0 && (reason))

#define assert(expr)                                                      \
	(expr) ? (void)0 : assert_failed(#expr, __FILE__, __LINE__, __FUNCTION__)

extern void assert_failed(char *statement, char *file, uint32_t line, const char *func);
extern void assert_failed_msg(char *file, uint32_t line, char *statement,
				     ...);

#define EXPAND_ARGS(...) __VA_ARGS__
#define fail_stmt(why, stmt)                                                   \
	assert_failed_msg(__FILE__, __LINE__, why, EXPAND_ARGS(stmt))

#define fail_stmt_stop(why, stmt)                                              \
	fail_stmt(why, stmt);                                                  \
	while (1)                                                              \
		;

void debug_print(char *message, ...);