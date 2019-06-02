#pragma once
#include <stdint.h>
#include <stdarg.h>

#define assert(expr) assert_msg(expr, "", "")
#define assert_msg(expr, format, ...)                                          \
	(expr) ? (void)0 :                                                     \
		 assert_failed(#expr, __FILE__, __LINE__, __FUNCTION__,        \
			       format, __VA_ARGS__)

extern void assert_failed(char *statement, char *file, uint32_t line,
			  const char *func, char *format, ...);