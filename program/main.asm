[org 0]
[bits 32]

[section .prog]
start:
        push ebx
        mov bl, 0x9e ; 9e

        xor ecx, ecx
        mov cl, 0x5b

        jmp short (scc + 1)

  scc:
        loop $ ; e2 fe
        retf
        ror bl, 2 ; works as the last two bits are 11
        mov al, bl ; 

        aam 0x12
        mul ah

        lahf
        xchg al, ah

        pushad
        mov esi, hw
        xor ah, ah
        mov al, 4
        int 0x33
        popad

;        pop edi
        pop ebx
        jmp scc

  hw:
        dd "Hello World!", 10
        times 0x40-($-$$) db 0
