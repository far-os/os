[bits 32]

[section .prog]
start:
        push ebx
        push edi
        mov bl, 0x9c ; 9c
        mov al, 0xc3 ; second half of inc bl, also ret

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
