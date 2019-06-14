#pragma once
#include "sys/types.h"

void LoadNewPageDirectory(size_t pd);
void DisablePSE();