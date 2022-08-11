;
; OS
;
[bits 16]
[org 0x7c00]
OFFSET equ 0x1a000 ; the offset at which our kernel is loaded

        mov [boot_drv], dl
        mov bl, [boot_drv]
        call print_hx_32_real

        mov bp, 0x6000 ; stack, remember it grows down
        mov sp, bp

        mov bx, string ; log a message
        call print_16

        call load_krn

        ; start protected!
        call prot

; GDT (Global Descriptor Table)
boot_drv: db 0

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

; we can't right away use cs and ds, so we have this disgrace
CODE equ gdt_code_seg - gdt_start
DATA equ gdt_data_seg - gdt_start

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
        
        jmp CODE:seg_init ; init the segments
        ret

print_16:
        pusha
        mov ah, 0x0e ; teletype mode - the character to print goes in al
  char_16:
        mov al, [bx] ; move what's at our string to al
        int 0x10 ; print
        inc bx ; move along the string

        cmp byte[bx], 0 ; is character null?
        jnz char_16 ; if not continue string
        popa
        ret
 
print_hx_32_real: ; prints hex string from ebx
        pusha
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
        mov edx, ebx ; move ebx into the temporary edx - we do the shift work here
        shr edx, cl ; shift by our amount - due to the cpu limitations this amount can only be in cx 
        and dl, 0x0F ; last nybble (since we discarded the ones after it this is the one we want)
        add dl, 0x30 ; push into the range of ascii numbers
        cmp dl, 0x3A ; for the letters - they will be outside of numbers
        jnge continue_32_real ; if it's a number go to continue
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
        mov bx, kernel_in_progress ; kernel boot message
        call print_16 

        mov cx, ((OFFSET >> 4) - 4) ; writes to the address
        mov es, cx ; puts the address in es, which is where the read interrupt looks

        mov bx, 0x0 ; where to put the sectors (with es)
        mov dh, 15 ; load 15 sectors
        mov dl, [boot_drv] ; load from the boot drive
        call read

        ret

read:
        push dx

        mov ah, 0x02 ; read sector
        mov al, dh ; read dh number of sectors
        mov ch, 0x00 ; cylinder #0
        mov dh, 0x00 ; head #0
        mov cl, 0x02 ; second sector (first after boot sector: includes csdfs superblock)

        int 0x13 ; read

        jc disk_fail ; in the event of failure

        pop dx       ; we pushed how many sectors we retrieved earlier 
        cmp dh, al   ; check to see if we got how many we asked for?
        jne disk_fail ; if not, fail
  write_disk_error:
        mov ebx, 0xd15c0000 ; blank out bx, add a "d15c" (for disk) so that we know it's the disk code
        mov bl, ah ; move the return status into bl
        call print_hx_32_real ; print return status

        ret
  disk_fail:
        mov bx, disk_error
        call print_16
        jmp write_disk_error
  disk_error:
        db "Disk read error!",0xd,0xa,0

string:
        db "Starting in Real Mode...",0xd,0xa,0
kernel_in_progress:
        db "Loading external kernel...",0xd,0xa,0
[bits 32]
seg_init:
        ; 32-bit segments
        mov ax, DATA ; data segment: moved to all other segment registers 
        mov ds, ax
        mov ss, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        mov ebp, 0x19f00 ; stack is now just behind superblock
        mov esp, ebp

        jmp OFFSET

        times 510-($-$$) db 0 ; pad to the 510th byte

        dw 0xaa55 ; magic number
