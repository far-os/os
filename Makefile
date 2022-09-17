.DEFAULT_GOAL := os.img
.PHONY: qemu bochs clean

# Disk size in units of 512KiB (Half MiB)
export DISK_SIZE_HM := 1
export KERN_SIZE := 0x20

boot.bin: boot.asm
	nasm $^ -f bin -o $@

entry.o: kernel/entry.asm
	nasm $^ -f elf -o $@

kernel.o: kernel/kernel.c $(wildcard kernel/*.h)
	gcc -falign-functions=1 -fno-stack-protector -ffreestanding -m32 -march=i686 -Wall -c $< -o $@

prog.o: program/main.asm
	nasm $^ -f elf -o $@

kernel.entry.o: link.ld entry.o kernel.o prog.o
	ld -o $@ -melf_i386 -T $+

kernel.bin: kernel.entry.o
	objcopy -O binary $^ $@

boot.kern.bin: boot.bin kernel.bin
	cat $^ > $@

os.img: boot.kern.bin
	dd if=/dev/zero of=$@ bs=512K count=$(DISK_SIZE_HM)
	dd if=$< of=$@ conv=notrunc

qemu: os.img
	$@-system-i386 -hda $< -boot c

bochs: os.img .bochsrc
	$@ -q

clean:
	rm -f *.bin *.o *.img
