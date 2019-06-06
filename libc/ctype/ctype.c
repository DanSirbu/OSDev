#include "ctype.h"

extern const int32_t **__ctype_tolower_loc(void);
extern const int32_t **__ctype_toupper_loc(void);
int tolower(int c)
{
	return c >= -128 && c < 256 ? (*__ctype_tolower_loc())[c] : c;
}
/* Return the uppercase version of C.  */
int toupper(int c)
{
	return c >= -128 && c < 256 ? (*__ctype_toupper_loc())[c] : c;
}