#include "fs.h"
#include "vfs.h"
#include "string.h"
#include "assert.h"
#include "kmalloc.h"

void test_tokenize()
{
	char *path = "/var/test/test2";

	char **tokens = tokenize(path);

	assert(strcmp("var", tokens[0]) == 0);
	assert(strcmp("test", tokens[1]) == 0);
	assert(strcmp("test2", tokens[2]) == 0);
	assert(tokens[3] == NULL);
	kfree_arr(tokens);

	char *path1 = "/var/test/test2/";

	char **tokens1 = tokenize(path1);

	assert(strcmp("var", tokens1[0]) == 0);
	assert(strcmp("test", tokens1[1]) == 0);
	assert(strcmp("test2", tokens1[2]) == 0);
	assert(tokens1[3] == NULL);
	kfree_arr(tokens1);

	char **tokens2 = tokenize("test");
	assert(strcmp("test", tokens2[0]) == 0);
	assert(tokens2[1] == NULL);
	kfree_arr(tokens2);

	char **tokens3 = tokenize("/");
	assert(strcmp("", tokens3[0]) == 0);
	assert(tokens3[1] == NULL);
	kfree_arr(tokens3);
}

inode_t *dummy_find_child(__attribute__((unused)) inode_t *cur,
			  __attribute__((unused)) char *name)
{
	return NULL;
}
void test_mount()
{
	inode_t *dummy_ino = kcalloc(sizeof(inode_t));
	dummy_ino->i_op = kcalloc(sizeof(inode_operations_t));
	dummy_ino->ino = 1;
	dummy_ino->i_op->find_child = dummy_find_child;

	assert(mount("/", dummy_ino) == 0);
	assert(mount("/abc", dummy_ino) == 0);
	assert(mount("/abc/hello", dummy_ino) == 0);
	assert(mount("/root/test", dummy_ino) < 0);

	assert(umount("/abc") == 0);

	assert(mount("/abc/hello2", dummy_ino) < 0);
}

void test_vfs()
{
	debug_print(" vfs:tokenize ");
	test_tokenize();
	/*debug_print(" vfs:mount ");
	test_mount();*/
}