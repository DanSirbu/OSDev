#include "list.h"
#include "spinlock.h"
#include "kmalloc.h"
#include "assert.h"

threaded_list_t *list_safe_create(void)
{
	return calloc(sizeof(threaded_list_t));
}
void list_safe_free(threaded_list_t *list)
{
	spinlock_acquire(&list->lock);
	node_t *n = list->head;
	while (n) {
		node_t *tmp = n->next;
		free(n);
		n = tmp;
	}
	list->len = 0; //Helps find use after free
	spinlock_release(&list->lock);
	free(list);
}

static void list_unsafe_append(threaded_list_t *list, node_t *node)
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
static void list_unsafe_remove(threaded_list_t *list, node_t *node)
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
void list_safe_enqueue(threaded_list_t *list, void *item)
{
	assert(list != NULL);
	assert(item != NULL);

	node_t *node = calloc(sizeof(node_t));
	node->value = item;
	spinlock_acquire(&list->lock);
	list_unsafe_append(list, node);
	spinlock_release(&list->lock);
}
void *list_safe_dequeue(threaded_list_t *list)
{
	spinlock_acquire(&list->lock);
	if (list->head == NULL || list->len == 0) {
		spinlock_release(&list->lock);
		return NULL;
	}
	void *item = list->head->value;
	list_unsafe_remove(list, list->head);
	spinlock_release(&list->lock);

	return item;
}