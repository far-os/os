        pushad

        xor ecx, ecx ; blank ecx - for looping later on
        mov bl, 0x5a ; magenta back, bright green fore

        xor eax, eax ; ah = 0, al = 5
        mov al, 5    ; the write string routine

        mov esi, cp  ; the string address
        int 0x33     ; writes string
        dec eax      ; al = 4, write character routine

        mov dx, 0xf01 ; dh = 0x0f, dl = 0x01
                      ; dl is set to 1 because it is the value needed to send a new line
                      ; dh's high nybble is use for the row counter, the low nybble
                      ; is used to gather the last nybble when anding it with bl: clever optimisation trick

        push eax ; save eax
        xor eax, eax ; ax = 0x0000 - advance cursor
        int 0x33 ; pushes cursor forward

        pop eax ; restore eax

        mov cl, 0x10 ; set loop iteration

  f_row:
        mov bl, cl  ; bl = 16 - cl
        neg bl      ; see below as to why this trick works

        call print_hex ; prints hex

        loop f_row

  reset_cl:
        mov cl, 0x10 ; set loop iterations

        dec eax  ; al = 3, insert special character routine
        int 0x33 ; inserts a new line
        inc eax  ; al = 4 again, write character routine

        mov bl, dh ; get high nybble of dh - the row counter
        shr bl, 4

        call print_hex ; prints hex

        mov bl, 0x6b ; yellow back, bright cyan fore

  p_row:
        mov bh, cl ; effectively bh = 16 - cl
        neg bh     ; if bh were, say 0xa, makes bh 0xf6 - note, we want 0x6 + whaterver row we are on
        and bh, dh ; dh serves as a row counter, with 0xf added on - if bh were 0xf6 and we were on row 5,
                   ; we would get 0x56 in bh - the high nybble being the row
                   ; and the low nybble being 16 - cl.

        int 0x33   ; print the value of bh to the screen

        loop p_row ; loops this for each column
        
        add dh, 0x10 ; increment the row counter (dh's high nybble)
        jnc reset_cl ; if the row counter hasn't surpassed 0xf, go on to next row

        popad
        jmp x_end ; return

print_hex:
        mov bh, bl
        and bh, 0x0f ; gets last nybble
        or bh, 0x30 ; move into numbers range
        cmp bh, 0x3a ; does it require a letter?

        jnae print_hex_cont ; if it's a number just move on

        add bh, 0x27 ; otherwise, shift it into the letters

  print_hex_cont:
        mov bl, 0x47 ; red back, white fore
        int 0x33 ; prints character
        
        ret

cp: db "  Code-page 437  ",0xa,0x0 ; heading
