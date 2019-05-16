include env.mk

# Makefile based on
# http://scottmcpeak.com/autodepend/autodepend.html

OBJDIR = obj

SRCFILES = $(wildcard boot/*.asm) $(wildcard kernel/*.c) $(wildcard lib/*.c) $(wildcard tests/*.c)
SRCFILES1 = $(patsubst %.c, $(OBJDIR)/%.o, $(SRCFILES))
OBJFILES = $(patsubst %.asm, $(OBJDIR)/%.o, $(SRCFILES1))

APPS-SRC = $(wildcard apps/*.c)
APPS-OBJ = $(patsubst %.c, $(OBJDIR)/%, $(APPS-SRC))

LIBC-SRC = $(wildcard libc/*.c) $(wildcard libc/*.S)
LIBC-SRC1 = $(patsubst %.c, $(OBJDIR)/%.o, $(LIBC-SRC))
LIBC-OBJ = $(patsubst %.S, $(OBJDIR)/%.o, $(LIBC-SRC1))

OBJDIRS:=$(dir $(OBJFILES)) $(dir $(APPS-OBJ)) $(dir $(LIBC-OBJ))
OBJDIRS:=$(shell echo $(OBJDIRS) | tr ' ' '\n' | uniq | tr '\n' ' ') # Keep unique only

CROSS-COMPILER:=clang#$(CROSS-COMPILER-DIR)/i686-elf-gcc
CROSS-LINKER:=$(CROSS-COMPILER-DIR)/i686-elf-ld

ARGS = -O0 -fno-pic -fno-stack-protector -g -nostdlib -ffreestanding -fno-common
ARGS += -Wall -Wextra -Werror -Wno-int-to-pointer-cast -m32
QEMU-ARGS = -no-shutdown -no-reboot -s -m 512M

GCC-APPS-ARGS = -fno-pic -fno-stack-protector -nostdlib -ffreestanding -fno-common -I./libc/ -m32

# -d int shows interrupts
QEMU-NETWORK-ARGS = -netdev type=user,id=network0,hostfwd=tcp::5555-:22,hostfwd=udp::5555-:22 -device rtl8139,netdev=network0 -object filter-dump,id=f1,netdev=network0,file=dump.pcap
#QEMU-NETWORK-ARGS = -netdev tap,helper=/usr/lib/qemu/qemu-bridge-helper,id=thor_net0 -device e1000,netdev=thor_net0,id=thor_nic0 -object filter-dump,id=f1,netdev=thor_net0,file=dump.pcap
#QEMU-NETWORK-ARGS = -net nic -net bridge,br=br0,id=netdevice -object filter-dump,id=f1,netdev=netdevice,file=dump.pcap

# Old style qemu network arguments
PORT7 = 5555
PORT80 = 5556
#QEMU-NETWORK-ARGS = -net user -net nic,model=rtl8139 -redir tcp:$(PORT7)::7 -redir tcp:$(PORT80)::80 -redir udp:$(PORT7)::7 -net dump,file=dump.pcap
#QEMU-NETWORK-ARGS = 
# By default os has ip 10.0.2.15
# Virtual Router has 10.0.2.2

#-monitor telnet:127.0.0.1:1235,server,nowait
run: os.iso
	$(QEMU-DIR)qemu-system-i386 $(QEMU-ARGS) -drive format=raw,file=$< -serial file:serial.log $(QEMU-NETWORK-ARGS) -vga std
	#@echo AAAAAAAAHello | ncat 127.0.0.1 5555 --send-only

run-debug: os.iso
	@$(QEMU-DIR)qemu-system-i386 $(QEMU-ARGS) -S -drive format=raw,file=$< -serial file:serial.log $(QEMU-NETWORK-ARGS) -vga std
	#@echo AAAAAAAAHello | ncat 127.0.0.1 5555 --send-only
	
debug-r2: $(OBJDIR)/kernel.elf
	r2 -e bin.baddr=0x001000000 -e dbg.exe.path=$< -d -b 32 -c v! gdb://127.0.0.1:1234
debug: $(OBJDIR)/kernel.elf obj/ramfs.img
	gdb $<

$(OBJDIR)/kernel.elf: ${OBJFILES}
	@$(CROSS-LINKER) -T link.ld $^ -o $@

obj/ramfs.img: ${APPS-OBJ}
	@./ramfs_gen $@ $^

$(OBJDIR)/%.o: %.c mkdirectories
	@$(CROSS-COMPILER) ${ARGS} $< -c -o $@ -I include/ -I tests/

$(OBJDIR)/%.o: %.asm mkdirectories
	@nasm -f elf32 -g $< -o $@

$(OBJDIR)/%.o: %.S mkdirectories
	@nasm -f elf32 -g $< -o $@

$(OBJDIR)/%: %.c mkdirectories ${LIBC-OBJ}
	@$(CROSS-COMPILER) ${GCC-APPS-ARGS} ${LIBC-OBJ} $< -o $@

libc: ${LIBC-OBJ}
	$(CROSS-LINKER) -r ${LIBC-OBJ} -o obj/libc/libcoraxlibc.a

mkdirectories:
	@mkdir -p $(OBJDIRS)
	
clean:
	rm -rf obj/
	rm -f serial.log
	rm -f .gdb_history

net-test:
	@echo AAAAAAAAHello | ncat 127.0.0.1 5555 --send-only
PHONY:
	$(info $$OBJFILES is [${OBJFILES}])

os.iso: $(OBJDIR)/kernel.elf obj/ramfs.img
	cp obj/kernel.elf isofiles/boot/kernel.elf
	cp obj/ramfs.img isofiles/boot/ramfs.img
	grub-mkrescue -o os.iso isofiles