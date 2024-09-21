        pushal

        xorl %ecx, %ecx
        incl %ecx    # make ecx = 1

        xorl %eax, %eax # ah = 1, al = 0
        incb %ah     # malloc

        int $0x33 # malloc

        movb $0x2d, %bl # green back, bright magenta fore

        call mem_put_addr

        pushl %edi

        int $0x33 # malloc

        call mem_put_addr

        # after malloc 2

        popl %edx # edx <- 1
        xchgl %edi, %edx # edx = 2, edi = 1

        movb $2, %al # realloc
        movl $0x30, %ecx # bigger

        int $0x33 # realloc

        xchgl %edi, %edx # edx = 1NEW, edi = 2
        call _mem_in_dx # the address is already in dx

        # freeing

        decb %al # free

        int $0x33

        movl %edx, %edi

        int $0x33

        popal

        jmp x_end

    mem_put_addr:
        movw %di, %dx
      _mem_in_dx:
        movb %dl, %bh

        pushl %eax
        shrw $6, %ax # 0x01xx -> 0x0004 (putch routine)

        int $0x33
        popl %eax

        ret

