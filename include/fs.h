#pragma once
#include "types.h"

//From James Molloy Tutorial

#define FS_FILE 0x01
#define FS_DIRECTORY 0x02
#define FS_CHARDEVICE 0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE 0x05
#define FS_SYMLINK 0x06
#define FS_MOUNTPOINT 0x08

struct fs_node;

//The directory stores child files like this (directory entry):
typedef struct {
	char name[128];
	uint32_t ino;
} dirent_t;


typedef uint32_t (*read_type_t)(struct fs_node *, uint32_t offset, uint32_t size,
				uint8_t *buf);
typedef uint32_t (*write_type_t)(struct fs_node *, uint32_t, uint32_t,
				 uint8_t *);
typedef void (*open_type_t)(struct fs_node *);
typedef void (*close_type_t)(struct fs_node *);
typedef dirent_t *(*readdir_type_t)(struct fs_node *, uint32_t);
typedef struct fs_node *(*finddir_type_t)(struct fs_node *, char *name);

typedef struct fs_node {
	char name[128]; // The filename.
	uint32_t ino;

	uint32_t flags; // Includes the node type (Directory, file etc).
	read_type_t read;
	write_type_t write;
	open_type_t open;
	close_type_t close;
	readdir_type_t readdir; // Returns the n'th child of a directory.
	finddir_type_t finddir; // Try to find a child in a directory by name.

	struct fs_node *symlink;
} fs_node_t;