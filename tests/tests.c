#include "test.h"

extern void test_tree();
void test_clone_directory();

void run_tests()
{
	test_tree();
	test_clone_directory();
}