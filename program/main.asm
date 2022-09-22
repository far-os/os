[org 0]
[bits 32]

[section .prog]
start:
        push ebx
        push edi
        mov bl, 0x9e ; 9e
        mov al, 0xcb ; second half of dec bl, also retf

        xor ecx, ecx
        mov cl, 2

        lea edi, [ecx+scc]

        stosb

        jmp short (scc + 1)

  scc:
        loop $ ; e2 fe
        resb 1 ; ret?
        ror bl, 2 ; works as the last two bits are 11
        mov al, bl ; 

        aam 0x12
        mul ah

        lahf
        xchg al, ah

        pop edi
        pop ebx
        or cl, 0x50
        jmp scc

        times 0x40-($-$$) db 0
