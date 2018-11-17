#pragma once
#include "types.h"

typedef struct {
	size_t edi;
	size_t esi;
	size_t ebx;
	size_t ebp;
	size_t eip;
} context_t;

void switch_context(context_t **cur_context, context_t *new_context);