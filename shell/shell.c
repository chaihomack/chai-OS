
#include "shell.h"

void start_shell()
{
    clear_screen();

    const char *prompt = "kernel_terminal: ";

    const unsigned int size_of_command;

    while (1)
    {
        kprint_newline();
        char command[size_of_command];
        kprint(prompt);
        
        kscanf(command, &size_of_command);
        kprint_newline();
        do_command(command);
    }
       
}