/* OUTPUT_FORMAT() flag */ 
ENTRY(sec_stage_start)

SECTIONS
 {
   . = 0x9000;
   .text : { *(.text) }
   .data : { *(.data) }
   .bss  : { *(.bss)  }

    . = ALIGN(512);     /* 4 bytes for magic 0x1BADB002 */
 }