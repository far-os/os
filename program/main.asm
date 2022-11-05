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

        lea eax, [j_table+edx*4] ; gets location in jump table
        bound eax, [j_table_data] ; test if eax is within bounds
        jmp dword [eax]


 
    col_test:
        %include "program/include/col.asm"

    cp_437:
        %include "program/include/cp437.asm"

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

  j_table_end:
