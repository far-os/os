.section .prog, "a"
.org 0

_bin_header: # file header
        magic: .ascii "FARb"  # magic number
        length: jmp _start     # length mov
        arch: .short 0xa86    # program architecture: ends in 86 for x86
                        # starts with 3 for 386, 4 for 486, etc until 6 for 686
                        # a for pentium with ia32
                        # 10 for pentium with x86_64
        error_handler: .long x_eh # error handler, return here with error code in ea

_start: 
        pushl %ebp
        movl %esp, %ebp

        movl 12(%ebp), %edx # retreive parameter - this is a far call so +12 not +8

        testl %edx, %edx
        jz com_amount
        decl %edx

        leal j_table(,%edx,4), %eax # gets location in jump table
        boundl %eax, j_table_data  # test if eax is within bounds
        jmp *(%eax)


    com_amount: 
        pushal

        movb $0x6b, %bl # lime fore, red back

        movl $j_table_end, %edx
        subl $j_table, %edx
        shrl $2, %edx

        movw $0x0100, %ax # malloc
        xorl %ecx, %ecx
        movb $16, %cl # 16 bytes
        int $0x33 # exec

        call write_n

        movl $com_amnt_str, %esi
        int $0x33

        movl $j_table_end, %edx # end of prog
        call write_n # write number (again - using same malloc'd buffer, will overwrite)

        movl $com_byt_str, %esi # byte string
        int $0x33

        movw $0x0101, %ax # free
        int $0x33

        popal
        jmp x_end

    write_n:  # moved out of function - reused code (calles itoa then write, takes edx in)
        movw $0x0200,%ax # itoa
        int $0x33 # exec

        movl %edi, %esi # will be able to write string
        xorw %ax, %ax
        movb $0x05, %al # string call
        int $0x33
        ret
    com_amnt_str: 
        .asciz " commands available\n"
    com_byt_str: 
        .asciz " bytes used"

    col_test: 
        .include "program/include/col.s"

    cp_437: 
        .include "program/include/cp437.s"

    mem_test: 
        .include "program/include/mem.s"

    mmx_out: 
        .include "program/include/mmx.s"

    death: 
        ud2
        jmp x_end

  x_end: 
        xorl %eax,%eax # no error

   x_eh: 
        leave
        lret

  j_table_data: 
        .long j_table
        .long j_table_end - 4
    j_table: 
        .long col_test
        .long cp_437
        .long mem_test
        .long mmx_out
        .long death

  j_table_end: 

