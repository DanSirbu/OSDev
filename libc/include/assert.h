#pragma once
#include <stdint.h>

#define assert(expr)                                                           \
	(expr) ? (void)0 :                                                     \
		 assert_failed(#expr, __FILE__, __LINE__, __FUNCTION__)

extern void assert_failed(char *statement, char *file, uint32_t line,
			  const char *func);