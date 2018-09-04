OBJFILES = boot/boot.o boot/idt.o lib/serial.o lib/string.o kernel.o
ARGS = -m32 -O0 -fno-pic -fno-stack-protector -g -nostdlib -ffreestanding

run: kernel.elf
	qemu-system-i386 -s -m 2G -kernel ./binary_x86/kernel.elf -serial file:serial.log
run-debug: kernel.elf
	qemu-system-i386 -s -S -m 2G -kernel ./binary_x86/kernel.elf -serial file:serial.log

debug:
	r2 -e bin.baddr=0x001000000 -e dbg.exe.path=/home/admin/Github/SmallKernel/binary_x86/kernel.elf -d -b 32 -c v! gdb://127.0.0.1:1234

kernel.elf: ${OBJFILES}
	ld -T link.ld -m elf_i386 $^ -o ./binary_x86/$@

%.o: %.c
	gcc ${ARGS} $< -c -o $@

%.o: %.asm
	nasm -f elf32 -g $<

clean:
	rm -f ${OBJFILES}
	rm -f ./binary_x86/kernel.elf
	rm -f serial.log
	rm -f .gdb_history


test: test.asm
	@nasm -f elf32 test.asm
	@clear
	@objdump -M intel -D -m i386 test.o | tail -n +5