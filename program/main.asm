[org 0]
[bits 32]

[section .prog]
start:
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

        mov bl, 0x6b ; yellow back, bright cyan fore

  reset_cl:
        mov cl, 0x10 ; set loop iterations
  p_row:
        mov bh, cl ; effectively bh = 16 - cl
        neg bh     ; if bh were, say 0xa, makes bh 0xf6 - note, we want 0x6 + whaterver row we are on
        and bh, dh ; dh serves as a row counter, with 0xf added on - if bh were 0xf6 and we were on row 5,
                   ; we would get 0x56 in bh - the high nybble being the row
                   ; and the low nybble being 16 - cl.

        int 0x33   ; print the value of bh to the screen

        loop p_row ; loops this for each column
        
        dec eax  ; al = 3, insert special character routine
        int 0x33 ; inserts a new line
        inc eax  ; al = 4 again, write character routine
        add dh, 0x10 ; increment the row counter (dh's high nybble)
        jnc reset_cl ; if the row counter hasn't surpassed 0xf, go on to next row

        popad
        retf ; return

  cp: db "  Codepage 437  ",0xa,0x0 ; heading
