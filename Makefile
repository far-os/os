.DEFAULT_GOAL := build/os.img
.PHONY: qemu bochs clean deepclean

_dummy := $(shell mkdir -p build)

# Partition size in units of 512KiB (Half MiB)
export DISK_SIZE_HM := 1

# Partition offset in sectors. For the sake of simplicity, we have here a partitionless disk for testing
export DISK_OFFSET := 0

# Size allocated to kernel in sectors. This must be able to fit boot.kern.bin, otherwise bad things will happen
# Also used to determine load location in memory (loaded at 0x80_000 - KERN_SIZE{in bytes}. done so that it resides in the highest possible region in 640k that's not possible hoarded by bios).
export KERN_SIZE := 100

CFLAGS := -falign-functions=1 -fno-stack-protector -ffreestanding -m32 -march=i686 -Wall -Werror=return-type -fpermissive -D"KERN_LEN=$(KERN_SIZE)"
CPPFLAGS := -fno-exceptions -fno-rtti -nostdinc++ $(CFLAGS)
# -static-libgcc. contains some long long arithmetic stuff

build/boot.bin: src/boot.asm
	nasm $^ -f bin -o $@

build/entry.o: src/kernel/entry.asm
	nasm $^ -f elf -o $@

build/apps.a: $(wildcard src/apps/*.cc) $(wildcard src/apps/extra/*.cc) $(wildcard src/apps/include/*.hh) $(wildcard src/apps/include/extra/*.hh)
	mkdir -p build/apps.obj
	cd build/apps.obj && g++ $(CPPFLAGS) -c ../../src/apps/*.cc
	cd build/apps.obj && g++ $(CPPFLAGS) -c ../../src/apps/extra/*.cc
	ar rv $@ build/apps.obj/*.o

build/kernel.a: $(wildcard src/kernel/*.c) $(wildcard src/kernel/include/*.h) $(wildcard src/kernel/syscall/*.h)
	mkdir -p build/kernel.obj
	cd build/kernel.obj && gcc $(CFLAGS) -c ../../src/kernel/*.c
	ar rv $@ build/kernel.obj/*.o

build/kernel.entry.o: src/link.ld build/entry.o build/kernel.a build/apps.a
	echo "kern_size = $(KERN_SIZE);" > build/env.ld
	ld -o $@ -melf_i386 -T $+

build/kernel.bin: build/kernel.entry.o
	objcopy -O binary $^ $@

build/prog.bin: src/program/main.s $(wildcard src/program/include/*.s)
	as $< --32 -o build/prog.o
	objcopy -O binary build/prog.o -j .prog $@

build/boot.kern.bin: build/boot.bin build/kernel.bin
	cat $^ > $@

./util/bin/qic:
	mkdir ./util/bin
	cargo build --manifest-path=./util/qic/Cargo.toml --release
	cp ./util/qic/target/release/qic ./util/bin

build/emptyfat.qi: ./util/bin/qic src/emptyfat.qit
	$^
	mv src/emptyfat.qi build

build/xconfig.qi: ./util/bin/qic src/xconfig.qit
	$^
	mv src/xconfig.qi build

build/os.img: build/boot.kern.bin build/emptyfat.qi build/xconfig.qi build/prog.bin $(wildcard files/*)
	$(if $(shell [ $$(($(KERN_SIZE)<<9)) -lt $$(stat -c %s build/boot.kern.bin) ] && echo "OK"), $(error FATAL: KERN_SIZE too small))
	dd if=/dev/zero of=$@ bs=512K count=$(DISK_SIZE_HM)
	dd if=$< of=$@ conv=notrunc
	dd if=build/emptyfat.qi of=$@ bs=512 conv=notrunc seek=$(KERN_SIZE)
	mcopy -i $@ build/prog.bin ::PROG.BIN
	mattrib -i $@ +r PROG.BIN
	mcopy -i $@ build/xconfig.qi ::XCONFIG.QI
	mattrib -i $@ +s XCONFIG.QI
	for f in `ls files` ; do echo "$$f" | awk '{print "::"toupper($$1)}' | xargs mcopy -i $@ files/$$f ; done
	chmod +w $@

qemu: build/os.img
	$@-system-i386 -hda $< -boot c

bochs: build/os.img .bochsrc
	$@ -q

clean:
	rm -rf ./build

deepclean: clean
	cargo clean --manifest-path=./util/qic/Cargo.toml
	rm -rf ./util/bin
