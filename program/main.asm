[org 0]
[bits 32]

[section .prog_h] ; file header
        magic: db "FARb" ; magic number
        length: jmp short end ; length mov
        arch: dw 0xa86  ; program architecture: ends in 86 for x86
                        ; starts with 3 for 386, 4 for 486, etc until 6 for 686
                        ; a for pentium with ia32
                        ; 10 for pentium with x86_64
        error_handler: dd x_eh ; error handler, return here with error code in ea
        end:


[section .prog]
start:
        push ebp
        mov ebp, esp

        mov edx, [ebp+12] ; retreive parameter - this is a far call so +12 not +8

        test edx, edx
        jz com_amount
        dec edx

        lea eax, [j_table+edx*4] ; gets location in jump table
        bound eax, [j_table_data] ; test if eax is within bounds
        jmp dword [eax]


    com_amount:
        pushad

        mov bl, 0x6b ; lime fore, red back

        mov edx, j_table_end
        sub edx, j_table
        shr edx, 2
        
        mov ax, 0x0100 ; malloc
        xor ecx, ecx
        mov cl, 16 ; 16 bytes
        int 0x33 ; exec

        call write_n

        mov esi, com_amnt_str
        int 0x33

        mov edx, j_table_end ; end of prog
        call write_n ; write number (again - using same malloc'd buffer, will overwrite)

        mov esi, com_byt_str ; byte string
        int 0x33

        mov ax, 0x0101 ; free
        int 0x33

        popad
        jmp x_end

    write_n: ; moved out of function - reused code (calles itoa then write, takes edx in)
        mov ax, 0x0200 ; itoa
        int 0x33 ; exec

        mov esi, edi ; will be able to write string
        xor ax, ax
        mov al, 0x05 ; string call
        int 0x33
        ret
    com_amnt_str:
        db " commands available", 0xa, 0x0
    com_byt_str:
        db " bytes used", 0x0
 
    col_test:
        %include "program/include/col.asm"

    cp_437:
        %include "program/include/cp437.asm"

    mem_test:
        %include "program/include/mem.asm"

  x_end:
        xor eax, eax ; no error
   
   x_eh:
        leave
        retf

  j_table_data:
        dd j_table         ; lower bound
        dd j_table_end - 4 ; upper bound - we subtract 4 because it wants the address of the last element
j_table:
        dd col_test
        dd cp_437
        dd mem_test

  j_table_end:
