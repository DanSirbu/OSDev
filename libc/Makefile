INCDIR = -I include -I ../include_common
OBJDIR = ../obj/libc


SRC = $(wildcard **/*.c) $(wildcard **/*.S)
OBJFILES = $(patsubst %, $(OBJDIR)/%, $(filter %.o, $(SRC:%.c=%.o) $(SRC:%.S=%.o)))

OBJDIRS:=$(shell echo $(dir $(OBJFILES)) | tr ' ' '\n' | uniq | tr '\n' ' ') # Keep unique only
$(info $(shell mkdir -p $(OBJDIRS))) # Make the directories

ARGS = -c -fno-pic -fno-stack-protector -nostdlib -ffreestanding -fno-common $(INCDIR) -m32 -O0 -g
ARGS += -Wall -Wextra -Wno-int-to-pointer-cast -m32 -std=c99

libcorax.a: $(OBJFILES)
	$(LD) -r $^ -o $(OBJDIR)/$@

$(OBJDIR)/%.o: %.c
	$(CC) $(ARGS) $< -o $@

$(OBJDIR)/%.o: %.S
	nasm -f elf32 -g $< -o $@