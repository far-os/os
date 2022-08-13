SECTIONS
{
  . = 0x19fc0;
  .text : { *(.text) }
  .data : { *(.data) }
  .rodata : { *(.rodata) }
}
