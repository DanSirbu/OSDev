#pragma once
#include "io.h"
#include "string.h"

#define HEX_PREFIX "0x"
void init_serial();
void kpanic_fmt(char *message, ...);
void kpanic(char *message);