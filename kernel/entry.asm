[bits 32]
[extern main]
kernel_entry:
        call main
        ret

%macro eh_macro 1 ; exception handler
global eh_%1
eh_%1:
;  push dword 0     ; no error code: so we use 0
;  push dword %1    ; push interrupt number
  jmp  generic_eh  ; generic eh
%endmacro

%macro e_eh_macro 1 ; exception handler w/ error code
global eh_%1
eh_%1:
;  push dword %1    ; push interrupt number
  jmp  generic_eh  ; generic eh
%endmacro

;[extern ih_c]
generic_eh:
        pushad ; save registers
;        call eh_c ; our handler
        popad ; restore register

;        add esp, 8 ; restore our stack: we pushed the error code and interrupt number
        iret       ; adios

eh_macro    0
eh_macro    1
eh_macro    2
eh_macro    3
eh_macro    4
eh_macro    5
eh_macro    6
eh_macro    7
e_eh_macro  8
eh_macro    9
e_eh_macro 10
e_eh_macro 11
e_eh_macro 12
e_eh_macro 13
e_eh_macro 14
eh_macro   15
eh_macro   16
e_eh_macro 17
eh_macro   18
eh_macro   19
eh_macro   20
eh_macro   21
eh_macro   22
eh_macro   23
eh_macro   24
eh_macro   25
eh_macro   26
eh_macro   27
eh_macro   28
eh_macro   29
e_eh_macro 30
eh_macro   31

global eh_list ; each one of the macros above
eh_list:
%assign i 0
%rep    32
    dd eh_%+i ; me when the the
%assign i i+1
%endrep
