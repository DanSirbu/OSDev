int test(int y)
{
	return y;
}
int main()
{
	int x = 1;
	while (1) {
		x = 0x123 * test(x);
	}
}
