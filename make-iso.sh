cp build/kernel/kernel.elf isofiles/boot/
cp build/ramfs.img isofiles/boot/
grub-mkrescue -o os.iso isofiles