#pragma once
#include "io.h"
#include "string.h"
#include <stdarg.h>

#define HEX_PREFIX "0x"
void init_serial();
void kpanic_fmt(char *message, ...);
void kpanic_fmt1(char *message, va_list args);
void kpanic(char *message);

#define assert(statement)                                                      \
	(statement) ? (void)0 : assert_failed(__FILE__, __LINE__, #statement)

#define fail_stop(why)                                                         \
	do {                                                                   \
		fail((why));                                                   \
		while (1)                                                      \
			;                                                      \
	} while (0)

#define fail(why) assert_failed(__FILE__, __LINE__, (why))

#define EXPAND_ARGS(...) __VA_ARGS__
#define fail_stmt(why, stmt)                                                   \
	assert_failed_msg(__FILE__, __LINE__, why, EXPAND_ARGS(stmt))

#define fail_stmt_stop(why, stmt)                                              \
	fail_stmt(why, stmt);                                                  \
	while (1)                                                              \
		;

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

static inline void assert_failed(char *file, uint32_t line,
				 const char *statement)
{
	kpanic_fmt("Error in %s:%d: " ANSI_COLOR_RED "%s" ANSI_COLOR_RESET "\n",
		   file, line, statement);
}
static inline void assert_failed_msg(char *file, uint32_t line, char *statement,
				     ...)
{
	kpanic_fmt("Error in %s:%d: \n", file, line);

	va_list args;
	va_start(args, statement);
	kpanic("\t");
	kpanic_fmt1(statement, args);
	kpanic("\n");
	va_end(args);
}