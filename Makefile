OBJFILES = boot.o idt.o kernel.o
ARGS = -m32 -O0 -fno-pic -fno-stack-protector -g -nostdlib

run: kernel.elf
	qemu-system-i386 -s -kernel kernel.elf -serial file:serial.log
debug:
	r2 -e bin.baddr=0x001000000 -e dbg.exe.path=/home/admin/Github/SmallKernel/kernel.elf -d -b 32 -c v! gdb://127.0.0.1:1234

kernel.elf: ${OBJFILES}
	ld -T link.ld -m elf_i386 $^ -o $@

%.o: %.c
	gcc ${ARGS} $< -c -o $@

%.o: %.asm
	nasm -f elf32 $<

clean:
	rm ${OBJFILES}

test: test.asm
	@nasm -f elf32 test.asm
	@clear
	@objdump -M intel -D -m i386 test.o | tail -n +5