;
; OS
;
[bits 16]
[org 0x7c00]
%define OFFSET 0x80000 - (%!KERN_SIZE << 9) ; the offset at which our kernel is loaded

%define MEMTAB_OFFS 0x8000
%define MEMTAB_MAX_ENTS 256 ; cannot have 256 or more entries, as that rolls over the one byte MEM_ENTS
%define MEMTAB_ENT_SIZE 24 ; entry size is 24 bytes. it does ask how many bytes we want for ome reason, so here is the number
%define CONFIG_OFFS 0xcc00

%define BOOT_DRV 0x0 ; the boot drive location, from gs
%define MEM_ENTS 0x1 ; the number of memory entries
%define DRV_PARAM 0x2 ; the boot drive parameter bock location, from gs. this is very very long, anything short should go before
%define KERN_LEN %!KERN_SIZE - 1 ; the kernel length. this changes quite frequently. MUST BE `4n-1`, as the FAT aftwerwards needs to be cluster-aligned. i decided 4 sectors per cluster

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
        fat_n_hidden_sectors: dd %!DISK_OFFSET ; number of sectors beforehand
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

        mov cx, (CONFIG_OFFS >> 4) ; 0xcc00 is where the data will be stored
        mov gs, cx

        mov bp, 0x6000 ; stack, remember it grows down
        mov sp, bp

        mov [gs:BOOT_DRV], dl
        mov [fat_drive_no], dl

        mov si, string ; log a message
        call print_16
        
        call mem ; deal with mem

        call load_krn
        
        ; fat_n_sectors is 16bit field, but if it's 0 look at the _extended field.
        ; for consistency's sake, we always make it zero and put it in _extended in runtime.
        movzx eax, word [fat_n_sectors]
        test ax, ax
        jz _after_sector_move

        mov [fat_n_sectors_extended], eax
        mov word [fat_n_sectors], 0


      _after_sector_move:

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

mem:
        mov cx, (MEMTAB_OFFS >> 4) ; segment 0x8000 where we want to put the memory
        mov es, cx
        mov di, 0 ; = 0x8000
        xor ebx, ebx
      .mem_loop:
        cmp di, (MEMTAB_ENT_SIZE * MEMTAB_MAX_ENTS) ; have we hit 256 entries? this is our maximum at which point we stop reading
        jae .mem_done ; we do this here as cmp trashes carry flag

        mov edx, "PAMS" ; SMAP. backwards because little endian
        mov eax, 0x0000e820 ; routine number
        mov ecx, MEMTAB_ENT_SIZE ; we copy 24 bytes.
        int 0x15 ; interrupt.

        test di, di ; check if this is the first time
        jnz .mem_after_first ; if we aren't first, skip preliminary checks
        jc  .mem_fail ; if carry flag set, fail
        cmp eax, "PAMS" ; is magic set?
        jne .mem_fail
      .mem_after_first:
        jcxz .mem_after_inc ; if we got no bytes out, don't bother incrementing di
        lea di, [di + MEMTAB_ENT_SIZE] ; advance di without changing carry (as we want to examine it later)
      .mem_after_inc:
        test ebx, ebx ; two terminating conditions: ebx=0, or cf=1 (hardware dependent)
        jna .mem_done ; !a == (cf=1) || (zf=1)
        jmp .mem_loop
      .mem_fail:
        ; do nothing, di is already 0
        ; can't set -1 here, as divide overflow throws #DE
      .mem_done:
        mov ax, di ; divide di by 24
        mov bl, 24
        div bl ; al = quotient
        mov [gs:MEM_ENTS], al ; load the length
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

;        call write_disk_error ; no longer have print_hx

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

        ret

  disk_fail:
        mov si, disk_error
        call print_16
        ret
;        jmp write_disk_error
  disk_error:
        db "! Disk read error",0xd,0xa,0

string:
        db "+ Starting",0xd,0xa,0
kernel_in_progress:
        db "+ Loading kernel",0xd,0xa,0
hdd_test:
        db "+ Reading disk",0xd,0xa,0
invalid_diskette:
        db 0xd,0xa,"! FarOS not found",0xd,0xa,0x0
dap_packet:
        dap_len: db 0x10 ; length of DAP
        reserved: db 0 ; is zero
        sect_amount: dw KERN_LEN ; amount of sectors
        seg_offset:
          dw 0x0 ; offset
          dw (OFFSET >> 4) ; segment
        seg_start: dq (%!DISK_OFFSET + 0x1) ; second sector (starts from zero - first after boot, including csdfs superblock)
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
