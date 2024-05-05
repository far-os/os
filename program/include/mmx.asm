        pushad

        mov ax, 0x0100
        xor ecx, ecx
        mov cl, 8 ; repeat eight times, and alloc eight bytes
        int 0x33

      mm_load_data_lp: ; creates 0x13_11_0f_0d_0b_09_07_05
        lea dx, [ecx * 2 + 3]
        mov [edi + ecx - 1], dl
        loop mm_load_data_lp

        pxor mm7, mm7 ; mm7 is zero

        movq mm0, [edi]
        pmaddwd mm0, mm0 ; multiply pairs and add together

        pcmpeqd mm2, mm2 ; compares both dwords in mm2 with itself - as it is equal, it fills with 1s
        psrlw mm2, 9 ; >> 9 - makes 0x00_7f_00_7f_00_7f_00_7f

        pand mm0, mm2 ; filter least significant 7 bits of bytes (% 128, gets ascii character)
        packuswb mm0, mm7 ; pack bytes - and place zeroes in high bits of each word

        movq [edi], mm0 ; load

        emms

        mov bl, 0x70 ; grey back black fore
        mov al, 0x05 ; 0x0005 - write string
        cbw ; movsx ax, al
        mov esi, edi ; string address
        int 0x33 ; put string, should be j+JN

        mov ax, 0x0101
        int 0x33 ; free

        popad

        jmp x_end
