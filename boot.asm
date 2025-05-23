;
; OS
;
[bits 16]
[org 0x7c00]
%define OFFSET 0x1a000 ; the offset at which our kernel is loaded

%define BOOT_DRV 0x0 ; the boot drive location, from gs
%define DRV_PARAM 0x1 ; the boot drive parameter bock location, from gs
%define KERN_LEN %!KERN_SIZE - 1 ; the kernel length. this changes quite frequently. MUST BE `4n-1`, as the FAT aftwerwards needs to be cluster-aligned. i decided 4 sectors per cluster

%define DISK_OFFSET 0
%define GDT_LEN 4
%define GDT_OFFS OFFSET + 0x1d0

        fat_jump: jmp short __start
        nop
        fat_oemid: db "FAROSfat"
        fat_bytes_per_sector: dw 512
        fat_sectors_per_cluster: db 4
        fat_rsrvd_sectors: dw %!KERN_SIZE
        fat_n_fats: db 2
        fat_n_root_entries: dw 0x100
        fat_n_sectors: dw (%!DISK_SIZE_HM * 1024) ; if DISK_SIZE_HM surpasses 64, we have problems
        fat_media_desc: db 0xf8 ; hard disk
        fat_sectors_per_fat: dw 4
        fat_sectors_per_track: dw 63
        fat_heads: dw 16
        fat_n_hidden_sectors: dd DISK_OFFSET ; number of sectors beforehand
        fat_n_sectors_extended: dd 0 ; if fat_n_sectors > 65535, use this.

        fat_drive_no: db 0x80 ; no idea really
        fat_nt_flags: db 0 ; mystery
        fat_sig: db 0x29 ; 0x28 or 0x29
        fat_vol_ser_no: dd 0xa30e4af3 ; volume serial number. whatever really
        fat_vol_label: db "FarOS Boot "
        fat_sys_id: db "FAT12   "

__start:
        xor cx, cx ; segment setup
        mov ds, cx
        mov es, cx
        mov ss, cx

        mov cx, 0xcc0 ; 0xcc00 is where the data will be stored
        mov gs, cx

        mov bp, 0x6000 ; stack, remember it grows down
        mov sp, bp

        mov [gs:BOOT_DRV], dl
        mov [fat_drive_no], dl

        mov si, string ; log a message
        call print_16

        call load_krn

        ; start protected!
        call prot


gdt_descriptor: ; gdt descriptor
        dw (0x8*(GDT_LEN+1)) - 1 ; GDT size
        dd GDT_OFFS ; where the gdt starts
prot:
        cli ; no more interrupts
        lgdt [gdt_descriptor] ; gdt time

        mov esi, cr0 ; set the protected mode flag
        or esi, 0x1  ; it can't be set directly, so we use
        mov cr0, esi ; esi a an intermediary register
        
        jmp 0x08:seg_init ; init the segments
        ret

print_16:
        pusha

        mov ah, 0x0f ; get page number in bh
        int 0x10

        mov ah, 0x0e; teletype mode - the character to print goes in al
  char_16:
        mov al, [si] ; move what's at our string to al
        int 0x10 ; print
        inc si ; move along the string

        cmp byte[si], 0 ; is character null?
        jnz char_16 ; if not continue string
        popa
        ret
 
load_krn:
        mov si, kernel_in_progress ; kernel boot message
        call print_16 

        mov cx, (OFFSET >> 4) ; writes to the address
        mov es, cx ; puts the address in es, which is where the read interrupt looks

        call read

        mov eax, 0xfa205d5c
        mov edi, 0x1fc ; load the magic number: its offset is already in es
        scasd
        jne krn_fail

        xor cx, cx
        mov es, cx

        ret

  krn_fail:
        mov si, invalid_diskette
        call print_16

        cli
        hlt

read:
        mov dl, [gs:BOOT_DRV]
        test dl, 0b10000000 ; are we on a hard disk?
        jnz read_hdd

    read_fdd:
        mov ah, 0x02 ; read sector
        mov al, KERN_LEN
        mov ch, 0x00 ; cylinder #0
        mov dh, 0x00 ; head #0
        mov cl, 0x02 ; second sector (first after boot sector: includes csdfs superblock)
        jmp r_end

    read_hdd:
        mov ah, 0x41 ; check to see if int 13 is supported
        mov bx, 0x55aa
        int 0x13
        jc read_fdd

        mov si, hdd_test
        call print_16
        mov ah, 0x42 ; read sector (extended edition)
        xor cx, cx
        mov ds, cx ; blank out ds
        mov si, dap_packet ; pin number

    r_end:
        int 0x13 ; read

        jc disk_fail ; in the event of failure

        call write_disk_error

        mov cx, gs ; put gs in ds
        mov ds, cx
        mov si, DRV_PARAM ; ds:si = 0cc0h:0001h

        mov byte [ds:si], 0x42 ; say how many bytes we want
        mov dl, [gs:BOOT_DRV] ; dl needs to contain drive index
        mov ah, 0x48 ; ah = 0x48, get ext. drive parameters
        int 0x13

        jc disk_fail

        xor cx, cx ; clean ds
        mov ds, cx

  write_disk_error:
        mov esi, 0xd15c0000 ; blank out bx, add a "d15c" (for disk) so that we know it's the disk code
        movzx si, ah ; move the return status into bl

        ret

  disk_fail:
        mov si, disk_error
        call print_16
        jmp write_disk_error
  disk_error:
        db "Disk read error!",0xd,0xa,0

string:
        db "Starting in Real Mode...",0xd,0xa,0
kernel_in_progress:
        db "Loading external kernel...",0xd,0xa,0
hdd_test:
        db "Reading from hard disk...",0xd,0xa,0
invalid_diskette:
        db 0xd,0xa,"FATAL: The disk does not contain a known file system.",0xd,0xa,0x0 
dap_packet:
        dap_len: db 0x10 ; length of DAP
        reserved: db 0 ; is zero
        sect_amount: dw KERN_LEN ; amount of sectors
        seg_offset:
          dw 0x0 ; offset
          dw (OFFSET >> 4) ; segment
        seg_start: dq (DISK_OFFSET + 0x1) ; second sector (starts from zero - first after boot, including csdfs superblock)
[bits 32]
seg_init:
        ; 32-bit segments
        mov ax, 0x10 ; data segment: moved to all other segment registers 
        mov ds, ax
        mov ss, ax
        mov es, ax

        mov ax, 0x20 ; external data segment

        mov fs, ax
        mov gs, ax

        mov ebp, (OFFSET - 0x100) ; stack is now just behind superblock
        mov esp, ebp

        jmp OFFSET

        times 510-($-$$) db 0 ; pad to the 510th byte

        dw 0xaa55 ; magic number
