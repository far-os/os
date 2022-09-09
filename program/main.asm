[org 0x1dd80]
[bits 32]

start:
        push ebx
        push edi
        mov bl, 0x9c ; 9c
        mov al, 0xc3 ; second half of inc bl, also ret

        mov edi, (scc + 2)
        stosb

        jmp short (scc + 1)

  scc:
        loop $ ; e2 fe
        resb 1 ; ret?
        ror bl, 2 ; works as the last two bits are 11
        mov al, bl ; 
        lahf
        xchg al, ah

        pop edi
        pop ebx
        mov ecx, 0x50
        jmp scc
