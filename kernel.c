#include "kernel.h"
#include "shell/shell.h"

void kmain() 
{
  idt_init();
  kb_init();

  clear_screen();
  
  start_shell();
  
}
