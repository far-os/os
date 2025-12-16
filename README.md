# FarOS
A simple OS with a kernel built in nasm and C, with applets written in C++.
To clone, makes sure you use `git clone --recursive`. See [below](#quick-information-compiler-qic)

## Compilation config
For ease in the emulation environment, the `Makefile` is by default configured to create a partitionless disk image, with size 512KiB.

This is controlled by the following `Makefile` variables:
| Variable | Default value | Meaning |
|:-:|:-|:-|
| `DISK_SIZE_HM` | 1 | The number of 512KiB blocks to create the partition image. Chosen to be quivalent to the value of cylinders, in an old-style CHS disk, assuming 16 heads and 63 sectors. |
| `DISK_OFFSET` | 0 | The LBA of the boot sector on disk, i.e. the sector offset of a partition, if you choose to contain the fie system in a partition. |
| `KERN_SIZE` | - | **DO NOT CHANGE!** The number of sectors allocated to the raw kernel, i.e. >= than the number of sectors taken up by `boot.kern.bin`. This must be a multiple of 4 sectors, so as to cluster align it in the file system. |

***NOTE:*** If you wish to install this to a partition of a disk, you must change at least `DISK_OFFSET` above.

## File hierarchy
The kernel code is located in `src/kernel/`, and the boot sector is located in `src/boot.asm`.

The kernel's source files are explained below: 
- `entry.asm`: The extended bootloader which calls `main`. It also contains the IDT as well.
- `ata.c`: Contains the ATA disk driver
- `cmos.c`: Functions for reading the time from the CMOS RTC
- `err.c`: Error message system - warnings, panics, etc
- `fs.c`: Contains the CSDFS driver.
- `hwinf.c`: Detecting and identifying detected hardware.
- `ih.c`: Interrupt handling functions.
- `kappldr.c`: For loading Kernel Applications (e.g. shell) in `kapps/`
- `kbd.c`: Utilities for reading input from the 8042 PS/2 device.
- `kernel.c`: The kernel itself.
- `memring.c`: Memory management: malloc and free
- `misc.c`: Definitions - structs, `#define`s, etc
- `printf.c`: Contains `printf` and its variations. TODO: move this into a stdlib
- `pic.c`: Utilities for initialising the PIC, masking the PIC, etc.
- `syscall.c`: Contains code for `syscall`ing.
- `text.c`: Code for writing to the screen.
- `timer.c`: Code to control the PIT.
- `util.c`: Miscellaneous utilities, such as `memcpy`, `strcpy`, `memcmp`, etc.

There is also the `src/syscall/` directory: each header file inside contains a different ABI service.

## Build instructions
You will need access to the following tools:

- `nasm`: To compile the bootloader sectors.
- `gcc`/`g++`: To compile the kernel.
- `ld`: To link the kernel code together with the extended bootloader sector.
- binutils: `ar`, `as`, `objcopy`: To imanage kernel's ELF object files/raw binaries.
- `cat`: To concatenate the bootloader and kernel binaries.
- `dd`: To create the disk image. **Note**: you will also need access to `/dev/zero` for this step.
- `make`: A convienent way to run all the commands.
- `mtools`: GNU toolchain that allows manipulation of FAT images
- `qic`: A custom tool for generating config files, see below

### Quick Information Compiler (qic)
`qic` is a custom utility for creating compiled configuration files. It is written in rust, so will require the rust toolchain to compile.
It is imported as a submodule - hence the need to clone recursively.

Once you have all of these, run `make`, and you will have an `os.img` file. This is the raw HDD disk image of the OS.

## Running
You can use any (supported) hardware or emulator, but the provided `Makefile` includes methods to use `bochs` or `qemu`. Both emulators work, however `qemu` is recommended, due to speed.

To build the image and use `bochs` or `qemu` simultaneously, run `make bochs` and `make qemu` respectively.
It also works on real hardware.
**Note**: the `.bochsrc` relies on an environment variable passed by the `Makefile`, so if using `bochs`, ***ONLY*** use it through the `Makefile`.

***WARNING: Keypresses may be different. The `.bochsrc` and OS natively are both configured to use the en-UK keyboard layout. Reconfiguring the `.bochsrc` should also fix the fact that the OS is configured for en-UK keypresses.***

## OS ABI
Programs in the sector directly after the kernel on disk will be copied to `0x100000` in memory: and will be executed in a segment where code will appear with origin `0`.

To syscall to the kernel, use `int 0x33`. The service code goes in `ah`, and the subroutine code goes in `al`. To see all services, see [the ABI docs](src/kernel/syscall/README.md).

### Program errors

| Error Code | Meaning |
|-|-|
| `0` | No error |
| `1` | Program error |
| `2` | Error caused externally |
| `3` | File inode invalid |
| `4` | Storage medium unavailable |
| `5` | Invalid address |
| `6` | Invalid/insufficient arguments |
| `7` | `bound` instruction failing (Program not found) |
| `9` | Illegal instruction (#UD exception) |
| `10` | Cannot kill process |
| `11` | Unknown value/command |
| `12` | Time error (e.g. invalid timezone, incorrect RTC values) |
| `15` | Program loading error, e.g. not a binary file |
| `18` | Error in configuration |
| `23` | Buffer space exceeded (Command too long) |
| `28` | Error in allocating handle (app handle, file handle, etc) |

