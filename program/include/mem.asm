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

        ; after malloc 2

        pop edx ; edx <- 1
        xchg edx, edi ; edx = 2, edi = 1

        xchg si, ax ; malloc 
        mov al, 2 ; realloc
        mov ecx, 0x30 ; bigger

        int 0x33

        xchg edx, edi ; edx = 1NEW, edi = 2
        mov bh, dl

        xchg si, ax ; write char

        int 0x33

        ; freeing

        xchg si, ax ; malloc
        dec al ; free

        int 0x33

        mov edi, edx

        int 0x33

        popad

        jmp x_end
        
