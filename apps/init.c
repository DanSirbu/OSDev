static inline int fork()
{
	int ret;
	asm("movl $1, %%eax; int $0x80; mov %%eax, %[retval]"
	    : [retval] "=r"(ret)
	    :
	    : "eax");

	return ret;
}
int test(int y)
{
	return y + 10;
}
void main()
{
	int x = 1;
	x = test(x) * 0x222;

	int y = fork();
	if (y == 0) {
		while (1) {
			int z = 0x999;
		}
	}
}