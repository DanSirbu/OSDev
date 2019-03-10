int test(int y)
{
	return y + 10;
}
int main()
{
	int x = 1;
	x = test(x) * 0x222;

	return x;
}