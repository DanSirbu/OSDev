#ifndef SCANF_H_
#define SCANF_H_

#include <stdarg.h>
#include <stdio.h>
#include <inttypes.h>

int vcbscanf(void *restrict cb_state, int (*getc_cb)(void *state),
	     void (*ungetc_cb)(void *state, int c), const char *restrict fmt,
	     va_list ap);
int cbscanf(void *restrict cb_state, int (*getc_cb)(void *state),
	    void (*ungetc_cb)(void *state, int c), const char *restrict fmt,
	    ...);

int fscanf(FILE *restrict f, const char *restrict fmt, ...);
int sscanf(const char *restrict s, const char *restrict fmt, ...);

float strtof(const char *restrict s, char **restrict p);
double strtod(const char *restrict s, char **restrict p);
long double strtold(const char *restrict s, char **restrict p);

unsigned long long strtoull(const char *restrict s, char **restrict p,
			    int base);
long long strtoll(const char *restrict s, char **restrict p, int base);
unsigned long strtoul(const char *restrict s, char **restrict p, int base);
long strtol(const char *restrict s, char **restrict p, int base);
intmax_t strtoimax(const char *restrict s, char **restrict p, int base);
uintmax_t strtoumax(const char *restrict s, char **restrict p, int base);

#endif
