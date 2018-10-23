#pragma once
#include "string.h"
#include "io.h"

#define HEX_PREFIX "0x"
void init_serial();
void kpanic_fmt(char *message, ...);
void kpanic(char *message);