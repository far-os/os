# OS
A simple OS built in nasm and C.

### File hierarchy
The kernel code is located in `kernel/`, and the boot sector is located in `boot.asm`

### To build
You will need access to the following tools:

- `nasm`: To compile to bootloader
- `gcc`: To compile the kernel
- `ld`: To link the kernel code and join it with an entrypoint
- `objcopy`: To convert the kernel's elf file into raw binary
- `cat`: To concatenate the bootloader and kernel binaries
- `make`: A convienent way to run all the commands

Once you have all of these, run `make`, and you will have an `os.bin` file. This is the floppy disk image of the OS. It may work if treated as a hard disk image, but from my testing it is very unlikely to load the kernel code.

### To run
You can use any hardware or emulator, but the provided `Makefile` includes methods to use `qemu` or `bochs`. Both emulators work, however `bochs` is recommended. You can use the provided `.bochsrc` file, or you can use your own.

To use `qemu` or `bochs` through the `Makefile`, run `make qemu` and `make bochs` respectively. 
