#pragma once
#include "string.h"

void init_serial();
void kpanic_fmt(char *message, ...);
void kpanic(char *message);