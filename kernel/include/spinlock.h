#pragma once
#include "sys/types.h"

void spinlock_acquire(int volatile *lock);
void spinlock_release(int volatile *lock);