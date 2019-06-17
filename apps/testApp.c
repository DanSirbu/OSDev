#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "sys/types.h"

int main(int argc, char *argv[])
{
	char *str = "abc";
	assert(strnlen(str, 9) == 3);
	assert(strnlen(str, 2) == 2);
	printf("TestApp\n");
}