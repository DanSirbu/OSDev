#pragma once
#include "sys/types.h"

//Based on ToaruOS Header file

typedef struct node {
	struct node *next;
	struct node *prev;
	void *value;
} __attribute__((packed)) node_t;

typedef struct {
	node_t *head;
	node_t *tail;
	size_t len;
} list_t;

list_t *list_create(void);
void list_merge(list_t *dst, list_t *src);
void list_remove(list_t *list, node_t *node);
void list_remove_item(list_t *list, void *item);
void list_free_contents(list_t *list);
void list_free(list_t *list);
node_t *list_find(list_t *list, void *value);
int list_index_of(list_t *list, void *value);
node_t *list_index(list_t *list, int item_index);

void list_enqueue(list_t *list, void *item);
void *list_dequeue(list_t *list);
void list_push(list_t *list, void *item);
void *list_pop(list_t *list);

#define foreach_list(list, n)                                                  \
	for (node_t *n = (list)->head; n != NULL; n = n->next)

/* Thread safe list below */
typedef struct {
	int lock;
	node_t *head;
	node_t *tail;
	size_t len;
} threaded_list_t;

threaded_list_t *list_safe_create(void);
void list_safe_free(threaded_list_t *list);

void list_safe_enqueue(threaded_list_t *list, void *item);
void *list_safe_dequeue(threaded_list_t *list);
/*void list_safe_push(threaded_list_t *list, void *item);
void *list_safe_pop(threaded_list_t *list);*/