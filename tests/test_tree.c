#include "serial.h"
#include "tree.h"

void test_tree()
{
	void *val1 = (void *)"/";
	void *val2 = (void *)"a";
	void *val3 = (void *)"b";
	void *val4 = (void *)"c";

	tree_node_t *root = tree_node_create(val1);
	tree_node_t *child1 = tree_node_insert_child(root, val2);
	tree_node_t *child2 = tree_node_insert_child(root, val3);

	assert(root->value == val1);

	//Make sure root has two children
	assert(child1->parent == root);
	assert(child2->parent == root);
	assert(list_find(root->children, (vptr_t)child1) != NULL);
	assert(list_find(root->children, (vptr_t)child2) != NULL);
	assert(root->children->len == 2);

	//Remove child1 temporarely
	tree_break_off(child1);
	assert(child1->parent == NULL);
	assert(list_find(root->children, (vptr_t)child1) == NULL);
	assert(list_find(root->children, (vptr_t)child2) != NULL);
	assert(root->children->len == 1);

	//Make sure the states the same as before after reinserting
	tree_node_insert_child_node(root, child1);
	assert(child1->parent == root);
	assert(child2->parent == root);
	assert(list_find(root->children, (vptr_t)child1) != NULL);
	assert(list_find(root->children, (vptr_t)child2) != NULL);
	assert(root->children->len == 2);

	//Test insert new value
	tree_node_t *child3 = tree_node_insert_child(child1, val4);
	assert(child1->children->len == 1);

	//Test remove node
	tree_remove_reparent_root(child1);
	//Tree should be / -> b, c
	assert(child3->parent == root);
	assert(list_index_of(root->children, (vptr_t)child3) != NULL);
}