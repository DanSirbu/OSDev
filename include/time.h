#pragma once
#include "sys/types.h"
#include "coraxstd.h"

void timer_init(uint32_t frequency);
int gettimeofday(struct timeval *p, void *z);