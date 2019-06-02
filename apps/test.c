int test(int y)
{
	return y + 10;
}
int main(int argc, char *argv[])
{
	int x = 1;
	x = test(x) * 0x222;
	for (int x = 0; x < argc; x++) {
		printf("Param: %s\n", argv[x]);
	}

	return x;
}