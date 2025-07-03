#include "shell/shell.h"
#include "drivers/disk_driver/disk_driver.h"
#include <stdint.h>
#include "drivers/keyboard/keyboard_driver.h"
#include "mylibs/my_stdlib.h"

uint32_t free_space_after_kernel_index;
int set_free_space_after_kernel_index()
{
  BYTE buffer[512];
  for (size_t i = 0; i < 99999999; i++)
    {
        ATA_disk_read(buffer, i, 1);
        if(*(uint32_t*)(buffer + 508) == 0xDEADBEEF){
            free_space_after_kernel_index = i + 1;
            return 0;
        }
    }
    return -1; // no signature
}

void kmain() 
{
  idt_init();
  kb_init();
  
  clear_screen();
  set_free_space_after_kernel_index();
  
  start_shell();  
  
}
