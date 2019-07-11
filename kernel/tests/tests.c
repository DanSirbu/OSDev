#include "test.h"
#include "debug.h"

extern void test_circularqueue();
extern void test_tree();
extern void test_clone_directory();
extern void test_vfs();

void dump_stack_trace_caller1()
{
	dump_stack_trace(NULL);
}
void dump_stack_trace_caller2()
{
	dump_stack_trace_caller1();
}

void run_tests()
{
	test_tree();
	test_clone_directory();
	test_vfs();
	test_circularqueue();
}