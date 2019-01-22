#include "test.h"

extern page_directory_t *current_directory;
void test_clone_directory()
{
	debug_print("Testing: clone directory\n");
	page_directory_t *newdir = clone_directory(current_directory);
	assert(newdir != current_directory);

	for (uint32_t x = 0; x < 1024; x++) {
		assert(newdir->actual_tables[x].present ==
		       current_directory->actual_tables[x].present);
		
		if(x >= KERN_BASE_PAGE_NO) {
			continue;
		}

		if (newdir->actual_tables[x].present) {
			assert(newdir->actual_tables[x].frame != current_directory->actual_tables[x].frame);
			for (int i = 0; i < 1024; i++) {
				assert(newdir->tables[x]->pages[i].present ==
				       current_directory->tables[x]
					       ->pages[i]
					       .present);
				assert(newdir->tables[x]->pages[i].frame != current_directory->tables[x]->pages[i].frame);
			}
		}
	}
}