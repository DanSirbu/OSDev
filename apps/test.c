#include <stdio.h>

int main(int argc, char *argv[])
{
	printf("Test!\n");
	for (int i = 0; i < argc; i++) {
		printf("Param: %d=%s\n", i, argv[i]);
	}

	return 0;
}