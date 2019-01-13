#pragma once
#include "types.h"

//Based on ToaruOS Header file

typedef struct node {
	struct node *next;
	struct node *prev;
	vptr_t value;
} __attribute__((packed)) node_t;

typedef struct {
	node_t *head;
	node_t *tail;
	size_t len;
} list_t;

void list_append(list_t *list, node_t *node);
node_t *list_append_item(list_t *list, vptr_t item);
list_t *list_create(void);
void list_merge(list_t *dst, list_t *src);
void list_remove(list_t *list, node_t *node);
void list_free_contents(list_t *list);
void list_free(list_t *list);
node_t *list_find(list_t *list, vptr_t value);
int list_index_of(list_t *list, vptr_t value);
node_t *list_index(list_t *list, int target_index);

#define foreach_list(list, n)                                            \
	for (node_t *n = (list)->head; n != NULL;                  \
	     n = n->next)
