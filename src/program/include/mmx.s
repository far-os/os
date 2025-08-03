        pushal

        movw $0x0100, %ax
        xorl %ecx, %ecx
        movb $8, %cl # repeat eight times, and alloc eight bytes
        int $0x33

      mm_load_data_lp:  # creates 0x13_11_0f_0d_0b_09_07_05
        leaw 0x3(,%ecx,2), %dx
        movb %dl, -1(%edi,%ecx,)
        loop mm_load_data_lp

        pxor %mm7, %mm7 # mm7 is zero

        movq (%edi), %mm0
        pmaddwd %mm0, %mm0 # multiply pairs and add together

        pcmpeqd %mm2, %mm2 # compares both dwords in mm2 with itself - as it is equal, it fills with 1s
        psrlw $9, %mm2 # >> 9 - makes 0x00_7f_00_7f_00_7f_00_7f

        pand %mm2, %mm0 # filter least significant 7 bits of bytes (% 128, gets ascii character)
        packuswb %mm7, %mm0 # pack bytes - and place zeroes in high bits of each word

        movq %mm0, (%edi) # load

        emms

        movb $0x70, %bl # grey back black fore
        movb $0x05, %al # 0x0005 - write string
        cbtw # movsbw %al, %ax
        movl %edi, %esi # string address
        int $0x33 # put string, should be j+JN

        movw $0x0101, %ax
        int $0x33 # free

        popal

        jmp x_end

