#include <stdio.h>
#include "assert.h"
#include "malloc.h"

void testApp()
{
	printf("Clone Test\n");
}
int main(int argc, char *argv[])
{
	printf("Test!\n");
	for (int i = 0; i < argc; i++) {
		printf("Param: %d=%s\n", i, argv[i]);
	}
	void *clone_addr = malloc(1000);
	assert(clone_addr != NULL);
	clone(testApp, clone_addr + 1000, NULL);
	return 0;
}