include env.mk

# Makefile based on
# http://scottmcpeak.com/autodepend/autodepend.html

OBJDIR = obj

SRCFILES = $(wildcard boot/*.asm) $(wildcard kernel/*.c) $(wildcard lib/*.c) $(wildcard tests/*.c)
SRCFILES1 = $(patsubst %.c, $(OBJDIR)/%.o, $(SRCFILES))
OBJFILES = $(patsubst %.asm, $(OBJDIR)/%.o, $(SRCFILES1))

APPS-SRC = $(wildcard apps/*.c)
APPS-OBJ = $(patsubst %.c, $(OBJDIR)/%, $(APPS-SRC))

OBJDIRS:=$(dir $(OBJFILES))
OBJDIRS:=$(shell echo $(OBJDIRS) | tr ' ' '\n' | uniq | tr '\n' ' ') # Keep unique only

APPS-OBJDIRS:=$(dir $(APPS-OBJ))
APPS-OBJDIRS:=$(shell echo $(APPS-OBJDIRS) | tr ' ' '\n' | uniq | tr '\n' ' ') # Keep unique only


CROSS-COMPILER:=$(CROSS-COMPILER-DIR)/i686-elf-gcc
CROSS-LINKER:=$(CROSS-COMPILER-DIR)/i686-elf-ld

ARGS = -O0 -fno-pic -fno-stack-protector -g -nostdlib -ffreestanding -fno-common
ARGS += -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Werror
QEMU-ARGS = -no-shutdown -no-reboot -s -m 512M

GCC-APPS-ARGS = -fno-pic -fno-stack-protector -nostdlib -ffreestanding -fno-common

# -d int shows interrupts
QEMU-NETWORK-ARGS = -netdev type=user,id=network0,hostfwd=tcp::5555-:22,hostfwd=udp::5555-:22 -device rtl8139,netdev=network0 -object filter-dump,id=f1,netdev=network0,file=dump.pcap
#QEMU-NETWORK-ARGS = -netdev tap,helper=/usr/lib/qemu/qemu-bridge-helper,id=thor_net0 -device e1000,netdev=thor_net0,id=thor_nic0 -object filter-dump,id=f1,netdev=thor_net0,file=dump.pcap
#QEMU-NETWORK-ARGS = -net nic -net bridge,br=br0,id=netdevice -object filter-dump,id=f1,netdev=netdevice,file=dump.pcap

QEMU-RAMFS = -initrd "obj/ramfs.img"
# Old style qemu network arguments
PORT7 = 5555
PORT80 = 5556
#QEMU-NETWORK-ARGS = -net user -net nic,model=rtl8139 -redir tcp:$(PORT7)::7 -redir tcp:$(PORT80)::80 -redir udp:$(PORT7)::7 -net dump,file=dump.pcap
#QEMU-NETWORK-ARGS = 
# By default os has ip 10.0.2.15
# Virtual Router has 10.0.2.2

#-monitor telnet:127.0.0.1:1235,server,nowait
run: $(OBJDIR)/kernel.elf obj/ramfs.img
	$(QEMU-DIR)qemu-system-i386 $(QEMU-ARGS) -kernel $< -serial file:serial.log $(QEMU-NETWORK-ARGS) $(QEMU-RAMFS)
	@echo AAAAAAAAHello | ncat 127.0.0.1 5555 --send-only

run-debug: $(OBJDIR)/kernel.elf
	@$(QEMU-DIR)qemu-system-i386 $(QEMU-ARGS) -S -kernel $< -serial file:serial.log $(QEMU-NETWORK-ARGS) $(QEMU-RAMFS)
	@echo AAAAAAAAHello | ncat 127.0.0.1 5555 --send-only
	
debug-r2: $(OBJDIR)/kernel.elf
	r2 -e bin.baddr=0x001000000 -e dbg.exe.path=$< -d -b 32 -c v! gdb://127.0.0.1:1234
debug: $(OBJDIR)/kernel.elf
	gdb $<

$(OBJDIR)/kernel.elf: ${OBJFILES}
	@$(CROSS-COMPILER) ${ARGS} -T link.ld $^ -o $@

obj/ramfs.img: ${APPS-OBJ}
	@./ramfs_gen $@ $^

stubstart.o: stubstart.S
	@nasm -f elf32 -g $< -o $@

$(OBJDIR)/%.o: %.c mkdirectories
	@$(CROSS-COMPILER) ${ARGS} $< -c -o $@ -I include/ -I tests/

$(OBJDIR)/%.o: %.asm mkdirectories
	@nasm -f elf32 -g $< -o $@

$(OBJDIR)/%: %.c mkdirectories
	@$(CROSS-COMPILER) ${GCC-APPS-ARGS} $< stubstart.o -o $@

mkdirectories:
	@mkdir -p $(OBJDIRS) $(APPS-OBJDIRS)
	
clean:
	rm -rf obj/
	rm -f serial.log
	rm -f .gdb_history

net-test:
	@echo AAAAAAAAHello | ncat 127.0.0.1 5555 --send-only
PHONY:
	$(info $$OBJFILES is [${OBJFILES}])
