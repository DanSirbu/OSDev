#include "fs.h"
#include "vfs.h"
#include "string.h"
#include "assert.h"

void test_tokenize()
{
	char *path = "/var/test/test2";

	char **tokens = tokenize(path);

	assert(strcmp("var", tokens[0]));
	assert(strcmp("test", tokens[1]));
	assert(strcmp("test2", tokens[2]));
	assert(tokens[3] == NULL);

	char *path1 = "/var/test/test2/";

	char **tokens1 = tokenize(path);

	assert(strcmp("var", tokens1[0]));
	assert(strcmp("test", tokens1[1]));
	assert(strcmp("test2", tokens1[2]));
	assert(tokens1[3] == NULL);
}

void test_vfs()
{
	test_tokenize();
}