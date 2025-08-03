        pushal

        movl $test_str, %esi

        movw $3, %ax # ax = 0x0003, insert special char
        movb $0b00000010, %dl # bit 1 = \r
        int $0x33

        movb $5, %al # ax = 0x0005, write string

        xorb %bl, %bl # bl = 0

        clc # clear carry

        xorl %ecx, %ecx # ecx = 0x0100, loop 256 times
        incb %ch

  f_loop: 
        int $0x33 # print

        addb $0x10, %bl # increment backgruond colour
        adcb $0x0, %bl # increment foreground colour, if background overflowed

        loop f_loop

  f_end: 
        popal
        jmp x_end


test_str: .asciz " abc " # five characters - fits 16 times across

