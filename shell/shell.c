
#include "shell.h"
#include "../drivers/keyboard/keyboard_driver.h"

uchar *prompt;  //extern from shell.h

void start_shell()
{
    clear_screen();

    prompt = "kernel_terminal: ";

    const unsigned int size_of_command;

    while (1)
    {
        kprint_newline();
        char command[size_of_command];
        kprint_str(prompt);
        
        kscanf(command, &size_of_command);
        kprint_newline();
        do_command(command);
    }
       
}