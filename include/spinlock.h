#pragma once
#include "sys/types.h"

void spinlock_acquire(int *lock);
void spinlock_release(int *lock);