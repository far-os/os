[org 0]
[bits 32]

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

        leave
        retf

    x_panic: db "panic!",0

  j_table_data:
        dd j_table         ; lower bound
        dd j_table_end - 4 ; upper bound - we subtract 4 because it wants the address of the last element
j_table:
        dd col_test
        dd cp_437

  j_table_end:
