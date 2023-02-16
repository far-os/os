        pushad

        xor ecx, ecx
        inc ecx      ; make ecx = 1
        
        xor eax, eax ; ah = 1, al = 0
        inc ah       ; malloc

        int 0x33

        mov dx, di
        mov bh, dl
        mov bl, 0x2d ; green back, bright magenta fore

        xor esi, esi ; ah = 0, al = 1
        mov si, 0x04 ; write char

        xchg si, ax ; write char
        
        int 0x33

        push edi

        xchg si, ax ; malloc

        int 0x33

        mov dx, di
        mov bh, dl

        xchg si, ax ; write char

        int 0x33

        xchg si, ax ; malloc
        inc al ; free

        int 0x33

        pop edi

        int 0x33

        popad

        jmp x_end
        
