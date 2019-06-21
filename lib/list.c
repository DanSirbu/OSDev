#include "list.h"
#include "kmalloc.h"
#include "assert.h"

#define malloc kmalloc
#define free kfree
#define calloc kcalloc

static void list_append(list_t *list, node_t *node)
{
	assert(list != NULL);
	assert(node != NULL);

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

	list->len++;
}
list_t *list_create(void)
{
	return calloc(sizeof(list_t));
}
void list_merge(list_t *dst, list_t *src)
{
	if (src->len == 0) {
		list_free(src);
		return;
	}

	if (dst->len == 0) {
		list_free(dst);
		dst = src;
		return;
	}

	dst->tail->next = src->head;
	src->head->prev = dst->tail;

	dst->len += src->len;

	list_free(src);
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
	node_t *next = node->next;

	if (prev) {
		prev->next = next;
	}
	if (next) {
		next->prev = prev;
	}

	node->prev = NULL;
	node->next = NULL;
	free(node);
	list->len--;
}
void list_remove_item(list_t *list, size_t item)
{
	node_t *node_item = list_find(list, item);
	assert(node_item !=
	       NULL); //TODO list_remove_item returns error if item not found
	list_remove(list, node_item);
}
void list_free_contents(list_t *list)
{
	node_t *n = list->head;
	while (n) {
		node_t *tmp = n->next;
		free((void *)n->value);
		n = tmp;
	}
}
void list_free(list_t *list)
{
	node_t *n = list->head;
	while (n) {
		node_t *tmp = n->next;
		free(n);
		n = tmp;
	}
	free(list);
}

node_t *list_find(list_t *list, size_t value)
{
	foreach_list(list, cur)
	{
		if (cur->value == value) {
			return cur;
		}
	}

	return NULL;
}
int list_index_of(list_t *list, size_t value)
{
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
void list_enqueue(list_t *list, void *item)
{
	assert(list != NULL);
	assert(item != NULL);

	node_t *node = calloc(sizeof(node_t));
	node->value = item;
	list_append(list, node);
}
void *list_dequeue(list_t *list)
{
	if (list->head == NULL || list->len == 0) {
		return NULL;
	}
	void *item = list->head->value;
	list_remove(list, list->head);

	return item;
}
void list_push(list_t *list, void *item)
{
	assert(list != NULL);
	assert(item != NULL);

	node_t *node = calloc(sizeof(node_t));
	node->value = item;

	if (list->len == 0) {
		list->head = node;
		list->tail = node;
		node->prev = NULL;
		node->next = NULL;
		list->len = 1;
		return;
	}
	node->next = list->head;
	node->prev = NULL;
	list->head = node;

	list->len++;
}
void *list_pop(list_t *list)
{
	return list_dequeue(list);
}