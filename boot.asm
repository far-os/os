;
; OS
;
[bits 16]
[org 0x7c00]
%define OFFSET 0x1a000 ; the offset at which our kernel is loaded
%define BOOT_DRV 0x0 ; the boot drive location, from gs
%define KERN_LEN %!KERN_SIZE - 1 ; the kernel length (15k kernel >:))

        xor cx, cx ; segment setup
        mov ds, cx
        mov es, cx
        mov ss, cx

        mov cx, 0xcc0 ; 0xcc00 is where the data will be stored
        mov gs, cx

        mov bp, 0x6000 ; stack, remember it grows down
        mov sp, bp

        mov [gs:BOOT_DRV], dl
        movzx esi, byte [gs:BOOT_DRV]
        call print_hx_32_real

        mov si, string ; log a message
        call print_16

        call load_krn

        ; start protected!
        call prot

gdt_start:
  gdt_null_desc: ; the mandatory null descriptor: eight null bytes
        dq 0x0

  gdt_code_seg: ; the segment in where code will be stored:
        ; base address = 0x0, size/limit = 0xffffff
        ; 1st flags: present in memory:1 privilege:00 descriptor type:1 => 1001b
        ; type flags: segment is code:1 conforming:0 readable:1 accessed:0 => 1010b
        ; 2nd flags granularity:1 32bit:1 64bit:0 AVL:0 = 1100b
        dw 0xffff ; limit (lower 16 bits)
        dw 0x0 ; base (lower 16 bits)
        db 0x0 ; base (bits 16-23)
        db 10011010b ; 1st flags and type flags
        db 11001111b ; 2nd flags and limit (upper 4 bits)
        db 0x0 ; base (upper 8 bits)

  gdt_data_seg:
        ; flags identical except for type flag segment is code:0
        dw 0xffff ; limit (lower 16 bits)
        dw 0x0 ; base (lower 16 bits)
        db 0x0 ; base (bits 16-23)
        db 10010010b ; 1st flags and type flags
        db 11001111b ; 2nd flags and limit (upper 4 bits)
        db 0x0 ; base (upper 8 bits)
  
  gdt_end:

gdt_descriptor: ; gdt descriptor
        dw gdt_end - gdt_start - 1 ; GDT size
        dd gdt_start ; where the gdt starts

prot:
        cli ; no more interrupts
        lgdt [gdt_descriptor] ; gdt time

        xor ebx, ebx
        call print_hx_32_real

        mov ebx, cr0 ; set the protected mode flag
        call print_hx_32_real
        or ebx, 0x1  ; it can't be set directly, so we use
        call print_hx_32_real
        mov cr0, ebx ; ebx as an intermediary register
        ;call print_hx_32_real
        
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
 
print_hx_32_real: ; prints hex string from ebx
        pusha
        mov ah, 0x0f ; page number
        int 0x10

        mov ah, 0x0e ; teletype mode
        mov cl, 0x20 ; length of ebx in bits
        mov al, 0x30 ; "0"
        int 0x10
        mov al, 0x78 ; "x"
        int 0x10
  print_nyb_32_real:
        ; this whole routine shifts ebx by a certain amount
        ; the amount starts at twenty-eight and decreases in increments of four (i.e. nybbles) all the way until zero
        ; that then provides each nybble, which is piped into continue

        sub cl, 4 ; subtract four
        mov edx, esi ; move esi into the temporary edx - we do the shift work here
        shr edx, cl ; shift by our amount - due to the cpu limitations this amount can only be in cx 
        and dl, 0x0F ; last nybble (since we discarded the ones after it this is the one we want)
        add dl, 0x30 ; push into the range of ascii numbers
        cmp dl, 0x3A ; for the letters - they will be outside of numbers
        jnae continue_32_real ; if it's a number go to continue
        add dl, 0x27 ; push the letters into the ascii lowercase letter range
  continue_32_real:
        mov al, dl ; moves our temporary dx into al, ready to print
        int 0x10 ; print

        cmp cl, 0 ; have we reached the end of ebx? - i.e. can we right shift no more after this
        jnz print_nyb_32_real ; if not, print the next nybble
        
        mov al, 0x0d ; print cr
        int 0x10
        mov al, 0x0a ; print lf
        int 0x10

        popa
        ret

load_krn:
        mov si, kernel_in_progress ; kernel boot message
        call print_16 

        mov cx, ((OFFSET >> 4) - 4) ; writes to the address
        mov es, cx ; puts the address in es, which is where the read interrupt looks

        call read

        mov eax, 0xac50dfc5 ; magic number
        xor edi, edi ; load the magic number: its address is alreaddy in es
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

  write_disk_error:
        mov esi, 0xd15c0000 ; blank out bx, add a "d15c" (for disk) so that we know it's the disk code
        movzx si, ah ; move the return status into bl
        call print_hx_32_real ; print return status

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
        db 0xd,0xa,"FATAL: The disk does not contain a valid CSDFS file system.",0xd,0xa,0x0 
dap_packet:
        dap_len: db 0x10 ; length of DAP
        reserved: db 0 ; is zero
        sect_amount: dw KERN_LEN ; amount of sectors
        seg_offset:
          dw 0x0 ; offset
          dw ((OFFSET >> 4) - 4) ; segment
        seg_start: dq 0x1 ; second sector (starts from zero - first after boot, including csdfs superblock)
[bits 32]
seg_init:
        ; 32-bit segments
        mov ax, 0x10 ; data segment: moved to all other segment registers 
        mov ds, ax
        mov ss, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        mov ebp, (OFFSET - 0x100) ; stack is now just behind superblock
        mov esp, ebp

        jmp OFFSET

        times 510-($-$$) db 0 ; pad to the 510th byte

        dw 0xaa55 ; magic number


