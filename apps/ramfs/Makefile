$(info $(shell mkdir -p build))

all: make_ram_file

make_ram_file: build/ramfs_gen build/readramfs
	build/ramfs_gen ../../build/ramfs.img ../../build/built_apps/*

build/ramfs_gen: ramfs_gen.c
	$(CC) $< -g -o $@

build/readramfs: readramfs.c
	$(CC) $< -g -o $@