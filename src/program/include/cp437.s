        pushal

        xorl %ecx, %ecx # blank ecx - for looping later on
        movb $0x5a, %bl # magenta back, bright green fore

        xorl %eax, %eax # ah = 0, al = 5
        movb $5, %al  # the write string routine

        movl $cp, %esi # the string address
        int $0x33    # writes string
        decl %eax    # al = 4, write character routine

        movw $0xf01, %dx # dh = 0x0f, dl = 0x01
                         # dl is set to 1 because it is the value needed to send a new line
                         # dh's high nybble is use for the row counter, the low nybble
                         # is used to gather the last nybble when anding it with bl: clever optimisation trick

        pushl %eax # save eax
        xorl %eax, %eax # ax = 0x0000 - advance cursor
        int $0x33 # pushes cursor forward

        popl %eax # restore eax

        movb $0x10, %cl # set loop iteration

  f_row: 
        movb %cl, %bl # bl = 16 - cl
        negb %bl    # see below as to why this trick works

        call print_hex # prints hex

        loop f_row

  reset_cl: 
        movb $0x10,%cl # set loop iterations

        decl %eax # al = 3, insert special character routine
        int $0x33 # inserts a new line
        incl %eax # al = 4 again, write character routine

        movb %dh, %bl # get high nybble of dh - the row counter
        shrb $4, %bl

        call print_hex # prints hex

        movb $0x6b, %bl # yellow back, bright cyan fore

  p_row: 
        movb %cl, %bh # effectively bh = 16 - cl
        negb %bh  # if bh were, say 0xa, makes bh 0xf6 - note, we want 0x6 + whaterver row we are on
        andb %dh, %bh # dh serves as a row counter, with 0xf added on - if bh were 0xf6 and we were on row 5,
                   # we would get 0x56 in bh - the high nybble being the row
                   # and the low nybble being 16 - cl.

        int $0x33  # print the value of bh to the screen

        loop p_row # loops this for each column

        addb $0x10, %dh # increment the row counter (dh's high nybble)
        jnc reset_cl # if the row counter hasn't surpassed 0xf, go on to next row

        popal
        jmp x_end # return

print_hex: 
        movb %bl, %bh
        andb $0x0f, %bh # gets last nybble
        orb $0x30, %bh # move into numbers range
        cmpb $0x3a, %bh # does it require a letter?

        jnae print_hex_cont # if it's a number just move on

        addb $0x27, %bh # otherwise, shift it into the letters

  print_hex_cont: 
        movb $0x47, %bl # red back, white fore
        int $0x33 # prints character

        ret

cp: .asciz "  Code-page 437  \n" # heading

