#include "syscalls.h"

int test(int y)
{
	return y + 10;
}
void main()
{
	int x = 1;
	x = test(x) * 0x222;

	int y = fork();
	if (y == 1) {
		while (1) {
			int z = 0x999;
		}
	}
}