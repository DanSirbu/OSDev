#pragma once
#include "fs.h"

typedef struct {
	uint32_t ino;
	char name[64];
} __attribute__((packed)) ramfs_dirent_t;

typedef struct {
	uint32_t num_dirs;
	ramfs_dirent_t *dirents;
} __attribute__((packed)) ramfs_dir_t;

typedef struct {
	uint32_t ino;
	uint8_t type;
	uint32_t address; /* Location of file contents */
	uint8_t read_only;
	uint32_t size; /* Size of file */
	uint32_t max_size; /* Maximum size allocated */
} __attribute__((packed)) ramfs_inode_t;

typedef struct {
	uint32_t numInodes;
	uint32_t max_inodes;
	ramfs_inode_t *inodes;
} __attribute__((packed)) ramfs_header_t; //Inodes