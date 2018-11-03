#pragma once
#include "io.h"
#include "string.h"

#define HEX_PREFIX "0x"
void init_serial();
void kpanic_fmt(char *message, ...);
void kpanic(char *message);

#define assert(statement)                                                      \
	(statement) ? (void)0 : assert_failed(__FILE__, __LINE__, #statement)

#define fail(why) assert_failed(__FILE__, __LINE__, why)

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

static inline void assert_failed(char *file, uint32_t line,
				 const char *statement)
{
	kpanic_fmt("Error in %s:%d: " ANSI_COLOR_RED "%s" ANSI_COLOR_RESET "\n",
		   file, line, statement);
}