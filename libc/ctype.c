#include "include/ctype.h"
#include <ctype.h>

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
	if (c == '\0') {
		return '\0';
	}
	const int32_t **tbl = __ctype_tolower_loc();
	int32_t ret = (*tbl)[c];
	if (ret == 0) {
		return c;
	}
	return ret;
}

/* Return the uppercase version of C.  */
int toupper(int c)
{
	if (c == '\0') {
		return '\0';
	}
	const int32_t **tbl = __ctype_toupper_loc();
	int32_t ret = (*tbl)[c];
	if (ret == 0) {
		return c;
	}
	return ret;
}
//__ctype_toupper_loc