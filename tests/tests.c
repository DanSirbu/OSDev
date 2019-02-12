#include "test.h"

extern void test_tree();
extern void test_clone_directory();
extern void test_vfs();
extern void test_ringbuffer();

void run_tests()
{
	test_tree();
	test_clone_directory();
	test_vfs();
	test_ringbuffer();
}