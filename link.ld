SECTIONS
{
  . = 0x19fc0;
  .csdfs : { *(.csdfs) }
  .second_boot : { *(.second_boot) }

  . = 0x1a1c0;
  .text : { *(.text) }
  .data : { *(.data) }
  .eh_frame : { *(.eh_frame) }
  .rodata : { *(.rodata) }
  .bss : { *(.bss) }
  /DISCARD/ : {
    *(.comment)
  }
}
