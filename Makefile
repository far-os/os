.DEFAULT_GOAL := os.img
.PHONY: qemu bochs clean

# Disk size in units of 512KiB (Half MiB)
export DISK_SIZE_HM := 1
export KERN_SIZE := 32

boot.bin: boot.asm
	nasm $^ -f bin -o $@

entry.o: kernel/entry.asm
	nasm $^ -f elf -o $@

kernel.o: kernel/kernel.c $(wildcard kernel/*.h) $(wildcard kernel/syscall/*.h)
	gcc -falign-functions=1 -fno-stack-protector -ffreestanding -m32 -march=i686 -Wall -c $< -o $@


kernel.entry.o: link.ld entry.o kernel.o
	ld -o $@ -melf_i386 -T $+

kernel.bin: kernel.entry.o
	objcopy -O binary $^ $@

prog.bin: program/main.asm
	nasm $^ -f bin -o $@

boot.kern.bin: boot.bin kernel.bin
	cat $^ > $@

os.img: boot.kern.bin prog.bin
	dd if=/dev/zero of=$@ bs=512K count=$(DISK_SIZE_HM)
	dd if=$< of=$@ conv=notrunc
	dd if=prog.bin of=$@ bs=512 conv=notrunc seek=$(KERN_SIZE)

qemu: os.img
	$@-system-i386 -hda $< -boot c

bochs: os.img .bochsrc
	$@ -q

clean:
	rm -f *.bin *.o *.img
