INCLUDE env.ld

loaded_at = 0x80000 - (kern_size << 9);
SECTIONS
{
  . = loaded_at;
  .second_boot : { *(.second_boot) }
  
  . = loaded_at + 0x1d0;
  .gdt : { *(.gdt) }

  . = loaded_at + 0x1fc;
  .magic : { *(.magic) }

  . = loaded_at + 0x200;
  .text : { *(.text) }
  .data : { *(.data) }
  .eh_frame : { *(.eh_frame) }
  .rodata : { *(.rodata) }
  .bss : { *(.bss) }

  /DISCARD/ : {
    *(.comment)
  }
}
