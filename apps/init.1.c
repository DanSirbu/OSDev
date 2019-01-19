int test(int y)
{
	return y + 10;
}
void main()
{
	int x = 1;
	x = test(x) * 0x222;
}