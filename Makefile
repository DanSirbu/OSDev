SRCFILES = $(wildcard boot/*.asm) $(wildcard lib/*.c) $(wildcard *.c)
SRCFILES1 = $(SRCFILES:.c=.o)
OBJFILES = $(SRCFILES1:.asm=.o)

ARGS = -m32 -O0 -fno-pic -fno-stack-protector -g -nostdlib -ffreestanding
ARGS += -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Werror
QEMU-ARGS = -no-shutdown -no-reboot -s -m 256M
# -d int shows interrupts
#QEMU-NETWORK-ARGS = -netdev type=user,id=network0,hostfwd=tcp::5555-:22 -device e1000,netdev=network0 -object filter-dump,id=f1,netdev=network0,file=dump.pcap
#QEMU-NETWORK-ARGS = -netdev tap,helper=/usr/lib/qemu/qemu-bridge-helper,id=thor_net0 -device e1000,netdev=thor_net0,id=thor_nic0 -object filter-dump,id=f1,netdev=thor_net0,file=dump.pcap
QEMU-NETWORK-ARGS = -net nic -net bridge,br=br0,id=netdevice -object filter-dump,id=f1,netdev=netdevice,file=dump.pcap
#QEMU-NETWORK-ARGS = 

run: kernel.elf
	@qemu-system-i386 $(QEMU-ARGS) -kernel ./binary_x86/kernel.elf -serial file:serial.log $(QEMU-NETWORK-ARGS)
run-debug: kernel.elf
	@qemu-system-i386 $(QEMU-ARGS) -S -kernel ./binary_x86/kernel.elf -serial file:serial.log $(QEMU-NETWORK-ARGS)

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

PHONY:
	$(info $$OBJFILES is [${OBJFILES}])
