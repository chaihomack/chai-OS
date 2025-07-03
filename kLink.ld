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

  . = ALIGN(512) - 4;     /* 4 bytes for magic 0xDEADBEEF */

  .footer : {
      LONG(0xDEADBEEF)    /* signature for kernel end idedntification */
  }
}
