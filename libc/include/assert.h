#pragma once
#include <stdint.h>

#define assert(expr)                                                           \
	(expr) ? (void)0 :                                                     \
		 assert_failed(#expr, __FILE__, __LINE__, __FUNCTION__)

//TODO add assert_failed in userland
#define assert_failed(unused, unused1, unused2, unused3) (void)0