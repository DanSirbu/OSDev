DEP-LIBS = ../obj/libc/libcorax.a

INCDIR = -I ../libc/include -I ../include_common
OBJDIR = ../obj/apps

CROSS-COMPILER-DIR=/home/admin/opt/cross/bin
CC:=$(CROSS-COMPILER-DIR)/i686-elf-gcc
CROSS-LINKER:=$(CROSS-COMPILER-DIR)/i686-elf-ld


SRC = $(wildcard *.c)
OBJFILES = $(patsubst %.c, $(OBJDIR)/%, $(SRC))

OBJDIRS:=$(shell echo $(dir $(OBJFILES)) | tr ' ' '\n' | uniq | tr '\n' ' ') # Keep unique only
$(info $(shell mkdir -p $(OBJDIRS))) # Make the directories


LIBS = -L ../obj/libc/ -lcorax -lgcc

ARGS = -fno-pic -fno-stack-protector -nostdlib -ffreestanding -fno-common -m32 -g
ARGS += $(INCDIR)


all: $(OBJDIR)/ramfs.img


$(OBJDIR)/%: %.c $(DEP-LIBS)
	$(CC) $(ARGS) $< $(LIBS) -o $@

$(OBJDIR)/ramfs.img: $(OBJFILES) ramfs/ramfs_gen
	ramfs/ramfs_gen $@ $(filter-out %/ramfs.img, $(shell echo $(OBJDIR)/*))

ramfs/ramfs_gen: ramfs/ramfs_gen.c
	gcc $< -o $@