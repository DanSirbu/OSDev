DEP-LIBS = ../obj/libc/libcorax.a

INCS = -I ../libc/include -I ../include_common
OBJDIR = ../obj/apps


SRC = $(wildcard *.c)
OBJFILES = $(patsubst %.c, $(OBJDIR)/%, $(SRC))

OBJDIRS:=$(shell echo $(dir $(OBJFILES)) | tr ' ' '\n' | uniq | tr '\n' ' ') # Keep unique only
$(info $(shell mkdir -p $(OBJDIRS))) # Make the directories


LIBS = -L ../obj/libc/ -lcorax -lgcc

ARGS = $(INCS) -fno-pic -fno-stack-protector -nostdlib -ffreestanding -fno-common -m32 -g -std=c99

all: $(OBJDIR)/ramfs.img


$(OBJDIR)/%: %.c $(DEP-LIBS)
	$(CC) $(ARGS) $< $(LIBS) -o $@

$(OBJDIR)/ramfs.img: $(OBJFILES) ramfs/ramfs_gen
	ramfs/ramfs_gen $@ $(filter-out %/ramfs.img, $(shell echo $(OBJDIR)/*))

ramfs/ramfs_gen: ramfs/ramfs_gen.c
	gcc $< -m32 -g -o $@