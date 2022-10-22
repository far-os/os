        pushad

        mov esi, test_str

        mov ax, 3 ; ax = 0x0003, insert special char
        mov dl, 0b00000010 ; bit 1 = \r
        int 0x33

        mov al, 5 ; ax = 0x0005, write string

        xor bl, bl ; bl = 0

        clc ; clear carry

        xor ecx, ecx ; ecx = 0x0100, loop 256 times
        inc ch

  f_loop:
        int 0x33 ; print

        add bl, 0x10 ; increment backgruond colour
        adc bl, 0x0  ; increment foreground colour, if background overflowed
        
        loop f_loop

  f_end:
        jmp x_end
        

test_str: db " abc ", 0 ; five characters - fits 16 times across
