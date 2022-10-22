# FarOS
A simple OS built in nasm and C.

## CSDFS
CSDFS (**C**ompact **S**ystem **D**isk **F**ile **S**ystem) is the file system used by FarOS disks. The bootable disks are organised like this, where each character is one sector:
`bBkkkkk...kkffffff....ff`
- `b` is the boot sector.
- `B` is the extended boot sector - this contains the CSDFS superblock (64 bytes) and more boot code.
- `k` - The first few sectors can be defined in the superblock to be non-FS: they tend to be used as kernel space, and a vfs file can be mapped to that block in memory.
- `f` - The actual file system.

## File hierarchy
The kernel code is located in `kernel/`, and the boot sector is located in `boot.asm`.

The kernel's files are explained below: 
- `ata.h`: Contains the ATA disk driver
- `defs.h`: Definitions - structs, `#define`s, etc
- `entry.asm`: The extended bootloader which calls `main`. It also contains the IDT as well.
- `fs.h`: Contains the CSDFS driver.
- `hwinf.h`: Detecting and identifying detected hardware.
- `ih.h`: Interrupt handling functions.
- `kbd.h`: Utilities for reading input from the 8042 PS/2 device.
- `kernel.c`: The kernel itself.
- `pic.h`: Utilities for initialising the PIC, masking the PIC, etc.
- `port.h`: Code to communicate to the I/O ports.
- `shell.h`: Contains FarSH, the shell.
- `syscall.h`: Contains code for `syscall`ing.
- `text.h`: Code for writing to the screen.
- `timer.h`: Code to control the PIT.
- `util.h`: Miscellaneous utilities, such as `memcpy`, `strcpy`, `memcmp`, etc.

There is also the `syscall/` directory: each header file inside contains a different ABI service.

## Build instructions
You will need access to the following tools:

- `nasm`: To compile the bootloader sectors.
- `gcc`: To compile the kernel.
- `ld`: To link the kernel code together with the extended bootloader sector.
- `objcopy`: To convert the kernel's ELF object into raw binary.
- `cat`: To concatenate the bootloader and kernel binaries.
- `dd`: To create the disk image. **Note**: you will also need access to `/dev/zero` for this step.
- `make`: A convienent way to run all the commands.

Once you have all of these, run `make`, and you will have an `os.img` file. This is the raw HDD disk image of the OS.

## Running
You can use any hardware or emulator, but the provided `Makefile` includes methods to use `bochs` or `qemu`. Both emulators *(should)* work, however `bochs` is recommended. You can use the provided `.bochsrc` file, or you can use your own.

To build the image and use `bochs` or `qemu` simultaneously, run `make bochs` and `make qemu` respectively.
It also works on real hardware.
**Note**: the `.bochsrc` relies on an environment variable passed by the `Makefile`, so if using `bochs`, ***ONLY*** use it through the `Makefile`.

***WARNING: Keypresses may be different. The `.bochsrc` and OS natively are both configured to use the en-UK keyboard layout. Reconfiguring the `.bochsrc` should also fix the fact that the OS is configured for en-UK keypresses.***

## OS ABI
Programs in the sector directly after the kernel on disk will be copied to `0x100000` in memory: and will be executed in a segment where code will appear with origin `0`.

To syscall to the kernel, use `int 0x33`. The service code goes in `ah`, and the subroutine code goes in `al`. To see all services, see [the ABI docs](kernel/syscall/README.md).

### Program errors

| Error Code | Meaning |
|-|-|
| `0` | No error |
| `1` | Program error |
| `2` | Error caused externally, e.g. a `bound` instruction failing |
