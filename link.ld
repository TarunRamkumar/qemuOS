ENTRY(_start)

SECTIONS {
  . = 0x80200000;

  .text : {
    *(.start)
    *(.text*)
  }

  .rodata : { *(.rodata*) }

  .data : { *(.data*) }

  .bss : {
    __bss_start = .;
    *(.bss*)
    __bss_end = .;
  }

  /DISCARD/ : {
    *(.comment)
    *(.note*)
  }
}
