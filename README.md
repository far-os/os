# OS
A simple OS built in nasm and C.

## File hierarchy
The kernel code is located in `kernel/`, and the boot sector is located in `boot.asm`.

The files are explained below: 
- `entry.asm`: The very first piece of kernel code loaded: its purpose is to call `main`, since `main` is not located at the start of the file.
- `kernel.c`: The kernel itself.
- `port.h`: Code to communicate to the I/O ports.
- `text.h`: Code for writing to the screen.

## Build instructions
You will need access to the following tools:

- `nasm`: To compile to bootloader
- `gcc`: To compile the kernel
- `ld`: To link the kernel code and join it with an entrypoint
- `objcopy`: To convert the kernel's elf file into raw binary
- `cat`: To concatenate the bootloader and kernel binaries
- `make`: A convienent way to run all the commands

Once you have all of these, run `make`, and you will have an `os.bin` file. This is the floppy disk image of the OS. It may work if treated as a hard disk image, but from my testing it is very unlikely to load the kernel code.

## Running
You can use any hardware or emulator, but the provided `Makefile` includes methods to use `qemu` or `bochs`. Both emulators work, however `bochs` is recommended. You can use the provided `.bochsrc` file, or you can use your own.

To use `qemu` or `bochs` through the `Makefile`, run `make qemu` and `make bochs` respectively. 
