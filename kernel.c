#include "kernel.h"
#include "mylibs/my_stdlib.h"
#include "shell/shell.h"
#include "drivers/keyboard/keyboard_driver.h"
#include "drivers/disk_driver/disk_driver.h"

void kmain() 
{
  idt_init();
  kb_init();

  clear_screen();
  
  ATA_disk_init();
  kprint_int(ATA_get_sector_count());
  
}
