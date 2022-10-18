[org 0]
[bits 32]

[section .prog]
start:
        push ebp
        mov ebp, esp

        mov edx, [ebp+12] ; retreive parameter - this is a far call so +12 not +8

        cmp edx, 1 ; check if calling function higher than length of table
        ja x_err   ; error
        
        
        mov eax, [j_table+edx*4] ; gets location in jump table
        jmp eax

    col_test:
        %include "program/include/col.asm"

    cp_437:
        %include "program/include/cp437.asm"

  x_err:
        mov eax, 1 ; error code 1

  x_end:
        leave
        retf

    x_panic: db "panic!",0

j_table:
        dd col_test
        dd cp_437
