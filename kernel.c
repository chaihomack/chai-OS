#include "kernel.h"
#include "drivers/disk_driver/disk_driver.h"

void kmain() 
{
  clear_screen();
  idt_init();
  kb_init();

  ATA_disk_initialize();
  kprint_newline();

  ATA_disk_write("haloo", 0, 1);
  kprint_newline();

  unsigned char strout[32];
  
  ATA_disk_read(strout, 0, 1);
  kprint_newline();
   
  kprint(strout);
  
}
