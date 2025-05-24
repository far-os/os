.DEFAULT_GOAL := os.img
.PHONY: qemu bochs clean deepclean

# Disk size in units of 512KiB (Half MiB)
export DISK_SIZE_HM := 1
export KERN_SIZE := 72
CFLAGS := -falign-functions=1 -fno-stack-protector -ffreestanding -m32 -march=i686 -Wall -fpermissive -D"KERN_LEN=$(KERN_SIZE)"

boot.bin: boot.asm
	nasm $^ -f bin -o $@

entry.o: kernel/entry.asm
	nasm $^ -f elf -o $@

kapps.o: kernel/kapps/kapp.cc $(wildcard kernel/kapps/*.hh)
	g++ -fno-exceptions -fno-rtti -nostdinc++ $(CFLAGS) -c $< -o $@

kernel.a: $(wildcard kernel/*.c) $(wildcard kernel/include/*.h) $(wildcard kernel/syscall/*.h) $(wildcard kernel/kapps/*.h)
	mkdir -p obj
	cd obj && gcc $(CFLAGS) -c ../kernel/*.c
	ar rv $@ obj/*.o

kernel.entry.o: link.ld entry.o kernel.a kapps.o
	echo "kern_size = $(KERN_SIZE);" > env.ld
	ld -o $@ -melf_i386 -T $+

kernel.bin: kernel.entry.o
	objcopy -O binary $^ $@

prog.bin: program/main.s $(wildcard program/include/*.s)
	as $< --32 -o prog.o
	objcopy -O binary prog.o -j .prog $@

boot.kern.bin: boot.bin kernel.bin
	cat $^ > $@

./util/bin/qic:
	mkdir ./util/bin
	cargo build --manifest-path=./util/qic/Cargo.toml --release
	cp ./util/qic/target/release/qic ./util/bin

emptyfat.qi: ./util/bin/qic emptyfat.qit
	$^

os.img: boot.kern.bin emptyfat.qi prog.bin program/data.txt
	dd if=/dev/zero of=$@ bs=512K count=$(DISK_SIZE_HM)
	dd if=$< of=$@ conv=notrunc
	dd if=emptyfat.qi of=$@ bs=512 conv=notrunc seek=$(KERN_SIZE)
	mcopy -i $@ prog.bin ::PROG.BIN
	mcopy -i $@ program/data.txt ::DATA.TXT
	chmod +w $@

qemu: os.img
	$@-system-i386 -hda $< -boot c

bochs: os.img .bochsrc
	$@ -q

clean:
	rm -rf ./obj
	rm -f *.qi *.bin *.o *.a *.img env.ld

deepclean: clean
	cargo clean --manifest-path=./util/qic/Cargo.toml
	rm -rf ./util/bin
