SECTIONS
{
  . = 0x19fc0;
  .csdfs : { *(.csdfs) }
  .text : { *(.text) }
  .data : { *(.data) }
  .rodata : { *(.rodata) }
}
