#include "kernel.h"
#include "mylibs/my_stdlib.h"
#include "shell/shell.h"
#include "drivers/keyboard/keyboard_driver.h"

void kmain() 
{
  idt_init();
  kb_init();

  clear_screen();
  kprint_int(1237881);
  
}
