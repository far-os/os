[section .csdfs]
[bits 32]
%define KERN_SIZE %!KERN_SIZE
%define DISK_SIZE_SEC %!DISK_SIZE_HM * (1 << 10) ; half mib * 1024

[section .second_boot]
[bits 32]
;
; Code
;
[extern main]
kernel_entry:
        mov ebx, protected ; log a message
        call print_32

        call a20_test
        
        push 0
        call clear_pag
        sub esp, 4

        call pre_main
        call main

        cli
        hlt

; todo: post_main using .fini_array
pre_main:
        ; c++ shit, stuff that needs to be called *before* main
        mov esi, kernel_entry+0x1c0
 _pre_main_loop:
        call [esi]

        add esi, 4
        cmp DWORD[esi], 0
        jnz _pre_main_loop
        ret


[global clear_pag]
clear_pag: ; clear screen
        push ebp ; c calling convention
        mov ebp, esp
        push edi
        
        mov edx, [ebp+8] ; page nr
        shl edx, 12

        mov ax, 0x0700
        mov ecx, (80 * 25)
        lea edi, [0xb8000 + edx]

        cld
        rep stosw

        pop edi
        leave
        ret

[global clear_pag_ln]
clear_pag_ln: ; void clear_pag_ln(unsigned char p, int lnr);
        push ebp ; c calling convention
        mov ebp, esp
        push edi
        
        mov edx, [ebp+12] ; load argument

        shl edx, 5 ; times by 32 - below it is times by 5 overall multiplying by 160

        mov ecx, [ebp+8] ; page nr
        shl ecx, 12

        lea edi, [edx * 4 + ecx + 0xb8000]
        add edi, edx
        mov ecx, 80
        mov ax, 0x0700

        cld
        rep stosw
        
        pop edi
        leave
        ret

[global scroll_pag]
scroll_pag:
        push ebp ; c calling convention
        mov ebp, esp
        pushad
        mov edx, [ebp+8] ; page nr
        shl edx, 12

        lea esi, [0xb80a0 + edx]
        lea edi, [0xb8000 + edx]
        mov ecx, (80 * 24 / 2)
        cld
        rep movsd

        push 24
        push edx
        call clear_pag_ln
        add esp, 8 ; pop what we just did

        popad

        leave
        ret

[global check_cpuid_avail]
check_cpuid_avail:
        pushfd         ; save eflags
        mov edx, [esp] ; backup eflags
        btc dword [esp], 21  ; change ID bit - it's only modifiable if cpuid is supported
        popfd          ; store eflags with changed bit

        pushfd         ; move eflags into eax
        pop eax        

        cmp eax, edx   ; has the eflags changed since the backup?
        setne al      ; if so, return 1
        
        ret


a20_test:
        pushad
        mov esi, 0x00200c ; even address
        mov edi, 0x10200c ; odd address, if no A20 should be the same as even address

        mov [esi], esi ; put different values at each address
        mov [edi], edi 

        cmpsd ; are they the values the same (only the case if no A20)
        jne a20_ret

[extern idle]
  a20_enable:
        in al, 0x64
        test al, 0b00000010
        jnz a20_ret

        call idle
        mov al, 0xd1
        out 0x64, al

        call idle
        mov al, 0xdf
        out 0x60, al
        
        call idle
  a20_ret:
        popad
        ret

[global fake_outsw]
fake_outsw:
        push ebp
        mov ebp, esp
        ; outsw function, but manual loops instead of rep
        pushad
        mov dx, [ebp + 0x8] ; port
        mov ecx, [ebp + 0xc] ; length
        mov esi, [ebp + 0x10] ; startpoint

        dec ecx

  fake_outsw_lp:
        outsw

        jmp $+2 ; timewaster, the reason we can't use rep
        nop

        dec ecx
        jns fake_outsw_lp

        popad
        leave
        ret

[global rip_thunk]
rip_thunk:
        pop eax
        jmp eax

print_32:
        pushad ; pusha but 32bit this time

        ; location of vram
        mov edi, 0xb8000 + (80 * 24 * 2) ; the last row of the 80x25 display
        xor ecx, ecx ; blank out counter
  char_32:
        mov al, [ebx + ecx] ; move what is in the string
        mov ah, 0x4a ; dark red back green fore
        mov [edi + 2 * ecx], ax ; place our character package (styles + character) in the vram

        inc ecx ; move along our string and vram

        cmp byte[ebx + ecx], 0x0 ; is character null?
        jnz char_32 ; if not, continue string
        popad ; popa but 32bit
        ret

protected:
        db "Successfully moved into Protected Mode!",0

[section .gdt]
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

  gdt_progc_seg:
        ; flags identical to code, except base is on the 1 MiB mark
        dw 0xffff ; limit (lower 16 bits)
        dw 0x0 ; base (low 16 bits)
        db 0x10 ; base (bits 16-23)
        db 10011010b ; 1st flags and type flags (access byte)
        db 11001111b ; 2nd flags and limit (upper 4 bits)
        db 0x0 ; base (upper 8 bits)

  gdt_progd_seg:
        ; flags identical to data, except base is on the 1 MiB mark
        dw 0xffff ; limit (lower 16 bits)
        dw 0x0 ; base (low 16 bits)
        db 0x10 ; base (bits 16-23)
        db 10010010b ; 1st flags and type flags (access byte)
        db 11001111b ; 2nd flags and limit (upper 4 bits)
        db 0x0 ; base (upper 8 bits)

[section .magic]
        dd 0xfa205d5c ; supposed to read farosdisc

[section .text]
[bits 32]

%macro eh_macro 1 ; exception handler
global eh_%1
eh_%1:
  push dword 0     ; no error code: so we use 0
  push dword %1    ; push interrupt number
  jmp  generic_eh  ; generic eh
%endmacro

%macro e_eh_macro 1 ; exception handler w/ error code
global eh_%1
eh_%1:
  push dword %1    ; push interrupt number
  jmp  generic_eh  ; generic eh
%endmacro

[global prog]
prog:
        push ebp
        mov ebp, esp
        
        mov dx, fs
        mov ds, dx
        mov es, dx

        ; test for magic
        push edi
        mov eax, "FARb"
        xor edi, edi
        scasd
        jne program_fail
        pop edi

        mov edx, [ebp+8] ; bring forward the parameter
        push edx
        call 0x18:4 ; will be jumped away
        pop edx

        mov dx, 0x10
        mov ds, dx
        mov es, dx

        jmp program_succ

 program_fail:
        pop edi
        mov al, 15 ; error code 0xf
        movzx eax, al
 program_succ:
        leave
        ret
        

[extern eh_c]
generic_eh:
        push ds
        push es

        pushad ; save registers

        mov ax, 0x10
        mov ds, ax
        mov es, ax

        call eh_c ; our handler
        popad

        pop es
        pop ds

        add esp, 8 ; restore our stack: we pushed the error code and interrupt number
        iretd       ; adios

eh_macro    0
eh_macro    1
eh_macro    2
eh_macro    3
eh_macro    4
eh_macro    5
eh_macro    6
eh_macro    7
e_eh_macro  8
eh_macro    9
e_eh_macro 10
e_eh_macro 11
e_eh_macro 12
e_eh_macro 13
e_eh_macro 14
eh_macro   15
eh_macro   16
e_eh_macro 17
eh_macro   18
eh_macro   19
eh_macro   20
eh_macro   21
eh_macro   22
eh_macro   23
eh_macro   24
eh_macro   25
eh_macro   26
eh_macro   27
eh_macro   28
eh_macro   29
e_eh_macro 30
eh_macro   31
eh_macro   32
eh_macro   33
eh_macro   34
eh_macro   35
eh_macro   36
eh_macro   37
eh_macro   38
eh_macro   39
eh_macro   40
eh_macro   41
eh_macro   42
eh_macro   43
eh_macro   44
eh_macro   45
eh_macro   46
eh_macro   47
eh_macro   48
eh_macro   49
eh_macro   50
eh_macro   51
eh_macro   52
eh_macro   53
eh_macro   54
eh_macro   55
eh_macro   56
eh_macro   57
eh_macro   58
eh_macro   59
eh_macro   60
eh_macro   61
eh_macro   62
eh_macro   63

global eh_list ; each one of the macros above
eh_list:
%assign i 0
%rep    64
    dd eh_%+i ; me when the the
%assign i i+1
%endrep
