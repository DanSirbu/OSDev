#include "tree.h"
#include "kmalloc.h"
#include "assert.h"

#define malloc kmalloc
#define free kfree
#define calloc kcalloc

tree_node_t *tree_node_create(void *value)
{
	tree_node_t *root = calloc(sizeof(tree_node_t));
	root->value = value;
	root->children = list_create();
	return root;
}

void tree_node_insert_child_node(tree_node_t *parent, tree_node_t *node)
{
	if (parent == NULL)
		return;

	node->parent = parent;

	list_append_item(parent->children, (size_t)node);
}

tree_node_t *tree_node_insert_child(tree_node_t *parent, void *value)
{
	tree_node_t *child = tree_node_create(value);
	tree_node_insert_child_node(parent, child);

	return child;
}
/* Removes node and moves all its children to the parent */
/* Note: does not free the value, you must do it yourself */
void tree_remove_reparent_root(tree_node_t *child)
{
	tree_node_t *parent = child->parent;
	assert(parent != NULL);

	list_remove(parent->children,
		    list_find(parent->children, (size_t)child));

	foreach_list(child->children, list_node)
	{
		tree_node_t *child = (tree_node_t *)list_node->value;
		child->parent = parent;
	}

	list_merge(parent->children, child->children);

	free(child);
}

tree_node_t *tree_find(tree_node_t *node, void *search_value,
		       decider_comparator_fn comparator)
{
	if ((*comparator)(node->value, search_value)) {
		return node;
	}

	tree_node_t *found;
	foreach_list(node->children, list_node)
	{
		tree_node_t *child_tree_node = (tree_node_t *)list_node->value;
		found = tree_find(child_tree_node, search_value, comparator);
		if (found)
			return found;
	}

	return NULL;
}
void tree_break_off(tree_node_t *node)
{
	tree_node_t *parent = node->parent;
	if (!parent)
		return;

	list_remove(parent->children,
		    list_find(parent->children, (size_t)node));
	node->parent = NULL;
}