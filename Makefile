include env.mk

SRCFILES = $(wildcard boot/*.asm) $(wildcard lib/*.c) $(wildcard *.c)
SRCFILES1 = $(SRCFILES:.c=.o)
OBJFILES = $(SRCFILES1:.asm=.o)

ARGS = -m32 -O0 -fno-pic -fno-stack-protector -g -nostdlib -ffreestanding
ARGS += -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Werror
QEMU-EXTRA-ARGS = 
QEMU-ARGS = -no-shutdown -no-reboot -s -m 256M
QEMU-ARGS += $(QEMU-EXTRA-ARGS)
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

run: kernel.elf
	$(QEMU-DIR)qemu-system-i386 $(QEMU-ARGS) -kernel ./binary_x86/kernel.elf -serial file:serial.log $(QEMU-NETWORK-ARGS)
	@echo AAAAAAAAHello | ncat 127.0.0.1 5555 --send-only

run-debug: kernel.elf
	@$(QEMU-DIR)qemu-system-i386 $(QEMU-ARGS) -S -kernel ./binary_x86/kernel.elf -serial file:serial.log $(QEMU-NETWORK-ARGS)
	@echo AAAAAAAAHello | ncat 127.0.0.1 5555 --send-only
	
debug-r2:
	r2 -e bin.baddr=0x001000000 -e dbg.exe.path=/home/admin/Github/SmallKernel/binary_x86/kernel.elf -d -b 32 -c v! gdb://127.0.0.1:1234
debug:
	gdb ./binary_x86/kernel.elf

kernel.elf: ${OBJFILES}
	@ld -T link.ld -m elf_i386 $^ -o ./binary_x86/$@

%.o: %.c
	@gcc ${ARGS} $< -c -o $@

%.o: %.asm
	@nasm -f elf32 -g $<

clean:
	rm -f ${OBJFILES}
	rm -f ./binary_x86/kernel.elf
	rm -f serial.log
	rm -f .gdb_history


test: test.asm
	@nasm -f elf32 test.asm
	@clear
	@objdump -M intel -D -m i386 test.o | tail -n +5
net-test:
	@echo AAAAAAAAHello | ncat 127.0.0.1 5555 --send-only
PHONY:
	$(info $$OBJFILES is [${OBJFILES}])
