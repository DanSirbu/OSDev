#include "list.h"
#include "kmalloc.h"

#define malloc kmalloc
#define free kfree
#define calloc kcalloc

void list_append(list_t *list, node_t *node)
{
	if (list->len == 0) {
		list->head = node;
		list->tail = node;
		node->prev = NULL;
		node->next = NULL;
		list->len = 1;
		return;
	}
	list->tail->next = node;
	node->prev = list->tail;
	list->tail = node;

	node->next = NULL;

	list->len++;
}
node_t *list_append_item(list_t *list, vptr_t item)
{
	node_t *node = malloc(sizeof(node_t));
	node->value = item;
	list_append(list, node);
	return node;
}
list_t *list_create(void)
{
	return calloc(sizeof(list_t));
}
void list_remove(list_t *list, node_t *node)
{
	if (list->head == node) {
		list->head = node->next;
	}
	if (list->tail == node) {
		list->tail = node->prev;
	}

	node_t *prev = node->prev;
	node->prev = node->next;
	node->next = prev;

	node->prev = NULL;
	node->next = NULL;
	list->len--;
}
void list_free(list_t *list)
{
	node_t *n = list->head;
	while (n) {
		node_t *tmp = n->next;
		free((void *)n->value);
		free(n);
		n = tmp;
	}
}
#define foreach_list(list, current)                                            \
	for (current = (list)->head; current != NULL; current = current->next)

node_t *list_find(list_t *list, vptr_t value)
{
	node_t *cur;
	foreach_list(list, cur)
	{
		if (cur->value == value) {
			return cur;
		}
	}

	return NULL;
}
int list_index_of(list_t *list, vptr_t value)
{
	node_t *cur;
	int index = 0;
	foreach_list(list, cur)
	{
		if (cur->value == value) {
			return index;
		}
		index++;
	}

	return -1;
}
node_t *list_index(list_t *list, int target_index)
{
	if (target_index == -1) {
		return NULL;
	}

	node_t *cur;
	int index = 0;
	foreach_list(list, cur)
	{
		if (index == target_index) {
			return cur;
		}
		index++;
	}

	return NULL;
}