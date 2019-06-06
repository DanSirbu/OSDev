#pragma once
#include "sys/types.h"

const unsigned short int **__ctype_b_loc(void);

extern int tolower(int __c);

extern int toupper(int __c);

/* Derived from newlib */
#define _U 01
#define _L 02
#define _N 04
#define _S 010
#define _P 020
#define _C 040
#define _X 0100
#define _B 0200