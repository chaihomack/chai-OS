/* OUTPUT_FORMAT() flag */
ENTRY(kstart)

SECTIONS
{
  . = 0x100000;
  .header : { LONG(0x1BADB002) }

  . = 0x100100;
  .text : {
    *(.text.startup)
    *(.text*)
  }

    .rodata : { *(.rodata*) }
  .data : { *(.data*) }

  .bss : {
      *(.bss)
      *(COMMON)
  }

  . = ALIGN(4096);
  .footer : {
      . += 4092;
      LONG(0xDEADBEEF)
  }

}
