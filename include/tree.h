#pragma once
#include "list.h"

//Based on the ToarusOS tree header

typedef int (*decider_comparator_fn)(void *, void *);

typedef struct tree_node {
	struct tree_node *parent;
	void *value;
	list_t *children;
} __attribute__((packed)) tree_node_t;

tree_node_t *tree_node_create(void *value);
tree_node_t *tree_node_insert_child(tree_node_t *parent, void *value);
void tree_remove_reparent_root(tree_node_t *node);
tree_node_t *tree_find(tree_node_t *node, void *search_value,
		       decider_comparator_fn comparator);
void tree_break_off(tree_node_t *node);