#include "include/ctype.h"

char _ctype_[256] = {
	_C,      _C,      _C,      _C,      _C,      _C,      _C,      _C,
	_C,      _C | _S, _C | _S, _C | _S, _C | _S, _C | _S, _C,      _C,
	_C,      _C,      _C,      _C,      _C,      _C,      _C,      _C,
	_C,      _C,      _C,      _C,      _C,      _C,      _C,      _C,
	_S | _B, _P,      _P,      _P,      _P,      _P,      _P,      _P,
	_P,      _P,      _P,      _P,      _P,      _P,      _P,      _P,
	_N,      _N,      _N,      _N,      _N,      _N,      _N,      _N,
	_N,      _N,      _P,      _P,      _P,      _P,      _P,      _P,
	_P,      _U | _X, _U | _X, _U | _X, _U | _X, _U | _X, _U | _X, _U,
	_U,      _U,      _U,      _U,      _U,      _U,      _U,      _U,
	_U,      _U,      _U,      _U,      _U,      _U,      _U,      _U,
	_U,      _U,      _U,      _P,      _P,      _P,      _P,      _P,
	_P,      _L | _X, _L | _X, _L | _X, _L | _X, _L | _X, _L | _X, _L,
	_L,      _L,      _L,      _L,      _L,      _L,      _L,      _L,
	_L,      _L,      _L,      _L,      _L,      _L,      _L,      _L,
	_L,      _L,      _L,      _P,      _P,      _P,      _P,      _C
};

const unsigned short int **__ctype_b_loc(void)
{
	return (const unsigned short int **)&_ctype_;
}

int tolower(int c)
{
	if (c >= 'A' && c <= 'Z') {
		return c - 'A' + 'a';
	}
	return c;
}

/* Return the uppercase version of C.  */
int toupper(int c)
{
	if (c >= 'a' && c <= 'z') {
		return c - 'a' + 'A';
	}
	return c;
}