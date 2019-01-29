#include "fs.h"
#include "mmu.h"
#include "assert.h"

typedef struct file_info {
	char name[64];
	void *start;
	uint32_t len;
} __attribute__((packed)) file_info_t;

file_info_t *headers;
uint32_t numfiles;

void initramfs(vptr_t location)
{
	/*ram_root = kcalloc(sizeof(fs_node_t));
	ram_root->readdir = initrd_readdir;
	ram_root->flags = FS_DIRECTORY;
	ram_root->read = 0;
	ram_root->write = 0;
	ram_root->open = 0;
	ram_root->close = 0;

	headers = (void *)location + sizeof(uint32_t);
	numfiles = *((uint32_t *)location);

	for (uint32_t x = 0; x < numfiles; x++) {
		headers[x].start += location; //Transform to virtual
	}*/
}
/*dirent_t *initrd_readdir(fs_node_t *node, uint32_t index)
{
	assert(node == ram_root);
	if (index > numfiles - 1) {
		return NULL;
	}

	strcpy(dirent.name, headers[index].name);
	dirent.ino = index;

	return &dirent;
}

uint32_t ramfs_read(file_t *node, uint8_t *buf, uint32_t size, uint32_t *offset)
{
	void *file_start = headers[node->ino].start;


    if(offset + size > headers[node->ino].len) {
        size = headers[node->ino].len - offset;
    }
    file_start += offset;

    memcpy(buf, file_start, size);

	return -1;
}
*/