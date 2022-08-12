SECTIONS
{
  . = 0x1a000;
  .text : { *(.text) }
  .data : { *(.data) }
  .rodata : { *(.rodata) }
}
