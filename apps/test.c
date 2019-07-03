#include <stdio.h>

int test(int y)
{
	return y + 10;
}
int main(int argc, char *argv[])
{
	int x = 1;
	x = test(x) * 0x222;
	for (int i = 0; i < argc; i++) {
		printf("Param: %d=%s\n", i, argv[x]);
	}

	return x;
}