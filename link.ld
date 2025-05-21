SECTIONS
{
  . = 0x1a000;
  .second_boot : { *(.second_boot) }
  
  . = 0x1a1d0;
  .gdt : { *(.gdt) }

  . = 0x1a1fc;
  .magic : { *(.magic) }

  . = 0x1a200;
  .text : { *(.text) }
  .data : { *(.data) }
  .eh_frame : { *(.eh_frame) }
  .rodata : { *(.rodata) }
  .bss : { *(.bss) }

  /DISCARD/ : {
    *(.comment)
  }
}
