#pragma once

typedef struct {
	uint32_t ino;
	char name[64];
} __attribute__((packed)) dir_dirent_t;

typedef struct {
	uint32_t num_dirs;
	void *unused;
} __attribute__((packed)) dir_header_t;