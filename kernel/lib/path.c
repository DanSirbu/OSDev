#include "list.h"
#include "vfs.h"
#include "string.h"
#include "path.h"
#include "kmalloc.h"

path_t *make_path(const char *raw_path)
{
	path_t *path = kmalloc(sizeof(path_t));
	path->items = list_create();

	add_path_items(path, raw_path);

	return path;
}

void free_path(path_t *path)
{
	if (path == NULL) {
		return;
	}
	char *pathItem = NULL;
	do {
		pathItem = list_dequeue(path->items);
		kfree(pathItem);
	} while (pathItem != NULL);

	kfree(path);
}
void add_path_items(path_t *path, const char *raw_path)
{
	char **tokens = tokenize(raw_path);
	for (size_t i = 0; i < array_length(tokens); i++) {
		list_enqueue(path->items, strdup(tokens[i]));
	}
	kfree_arr(tokens);
}
void move_up(path_t *path)
{
	kfree(list_dequeue_tail(path->items));
}
/*
 * Note: this path must be freed
 */
char *get_absolute_path(path_t *path)
{
	size_t len = 0;
	foreach_list(path->items, item)
	{
		len += 1; // / separator
		char *pathToken = item->value;
		len += strlen(pathToken);
	}
	len += 1; //Null terminator

	char *absolute_path = kmalloc(len);
	absolute_path[0] = 0;
	foreach_list(path->items, item)
	{
		strcat(absolute_path, "/");
		char *pathToken = item->value;
		strcat(absolute_path, pathToken);
	}

	return absolute_path;
}