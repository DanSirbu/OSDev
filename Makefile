# Change these variables to match your configuration
CROSS-COMPILER-DIR=/home/admin/opt/cross/bin


CC:=$(CROSS-COMPILER-DIR)/i686-elf-gcc
CROSS-LINKER:=$(CROSS-COMPILER-DIR)/i686-elf-ld
QEMU := qemu-system-i386


INCDIR = -I include -I include_common
OBJDIR = obj


SRC = $(wildcard boot/*.asm) $(wildcard kernel/*.c) $(wildcard lib/*.c) $(wildcard tests/*.c)
OBJFILES = $(patsubst %, $(OBJDIR)/%, $(filter %.o, $(SRC:%.c=%.o) $(SRC:%.asm=%.o)))

OBJDIRS:=$(shell echo $(dir $(OBJFILES)) | tr ' ' '\n' | uniq | tr '\n' ' ') # Keep unique only
$(info $(shell mkdir -p $(OBJDIRS))) # Make the directories

ARGS= $(INCDIR) -O0 -fno-pic -fno-stack-protector -g -nostdlib -ffreestanding -fno-common -Wall -Wextra -Wno-int-to-pointer-cast -m32 -std=c99
# 28/05/2019: I feel honoured, I've hit a qemu bug (#1748296 on launchpad) instead of my own bugs.
# shlx does not work unless we use the kvm
QEMU-ARGS=-no-shutdown -no-reboot -s -m 512M -serial file:serial.log -drive format=raw,file=$< -vga std

run: os.iso
	$(QEMU) $(QEMU-ARGS)

run-debug: os.iso
	$(QEMU) -S $(QEMU-ARGS)
	
debug:
	gdb $(OBJDIR)/kernel.elf

os.iso: $(OBJDIR)/kernel.elf libc apps
	cp obj/kernel.elf isofiles/boot/kernel.elf
	cp obj/apps/ramfs.img isofiles/boot/ramfs.img
	grub-mkrescue -o $@ isofiles

libc:
	$(MAKE) -C $@
apps:
	$(MAKE) -C $@

$(OBJDIR)/kernel.elf: ${OBJFILES}
	$(CROSS-LINKER) -T link.ld $^ -o $@

$(OBJDIR)/%.o: %.c
	$(CC) $(ARGS) $< -c -o $@ -I include/ -I tests/

$(OBJDIR)/%.o: %.asm
	nasm -f elf32 -g $< -o $@

$(OBJDIR)/%.o: %.S
	nasm -f elf32 -g $< -o $@
	
clean:
	rm -rf obj/
	rm -f serial.log
	rm -f .gdb_history


.PHONY: libc apps