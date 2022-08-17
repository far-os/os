# FarOS
A simple OS built in nasm and C.

## CSDFS
CSDFS (**C**ompact **S**ystem **D**isk **F**ile **S**ystem) is the file system used by FarOS diskettes. The bootable diskettes are organised like this, where each character is one sector:
`bBkkkkk...kkffffff....ff`
- `b` is the boot sector.
- `B` is the extended boot sector - this contains the CSDFS superblock (64 bytes) and more boot code.
- `k` - The first few sectors can be defined in the superblock to be non-FS: they tend to be used as kernel space, and a vfs file can be mapped to that block in memory.
- `f` - The actual file system.

## File hierarchy
The kernel code is located in `kernel/`, and the boot sector is located in `boot.asm`.

The kernel's files are explained below: 
- `entry.asm`: The extended bootloader which calls `main`. It also contains the IDT as well.
- `fs.h`: Contains the CSDFS driver.
- `ih.h`: Interrupt handling functions.
- `kbd.h`: Utilities for reading input from the 8042 PS/2 device.
- `kernel.c`: The kernel itself.
- `pic.h`: Utilities for initialising the PIC, masking the PIC, etc.
- `port.h`: Code to communicate to the I/O ports.
- `shell.h`: Contains FarSH, the shell.
- `text.h`: Code for writing to the screen.
- `util.h`: Miscellaneous utilities, such as `memcpy`, `strcpy`, `memcmp`, etc.

## Build instructions
You will need access to the following tools:

- `nasm`: To compile the bootloader sectors.
- `gcc`: To compile the kernel.
- `ld`: To link the kernel code together with the extended bootloader sector.
- `objcopy`: To convert the kernel's ELF object into raw binary.
- `cat`: To concatenate the bootloader and kernel binaries.
- `make`: A convienent way to run all the commands.

Once you have all of these, run `make`, and you will have an `os.bin` file. This is the floppy disk image of the OS.

## Running
You can use any hardware or emulator, but the provided `Makefile` includes methods to use `bochs` or `qemu`. Both emulators *(should)* work, however `bochs` is recommended. You can use the provided `.bochsrc` file, or you can use your own.

To build the image and use `bochs` or `qemu` simultaneously, run `make bochs` and `make qemu` respectively.

***WARNING: Keypresses may be different. The `.bochsrc` and OS natively are both configured to use the en-UK keyboard layout. Reconfiguring the `.bochsrc` should also fix the fact that the OS is configured for en-UK keypresses.***
