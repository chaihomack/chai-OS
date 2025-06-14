
#include "shell.h"
#include "../drivers/keyboard/keyboard_driver.h"

Prompt prompt = 
{
    .pwd = '/',
    .text_af_pwd = " : "
}; 

void start_shell()
{
    clear_screen();

    const uint32_t size_of_command = 512;

    while (1)
    {
        kprint_newline();
        uchar command[size_of_command];
        kprint_str(prompt.pwd);
        kprint_str(prompt.text_af_pwd);
        
        kscanf(command, &size_of_command);
        kprint_newline();
        do_command(command);
    }
       
}

void add_dir_in_prompt(const uchar* dir_name)
{
    size_t pwd_len = strlen(prompt.pwd);
    size_t name_len = strlen(dir_name);
    size_t total_len = pwd_len + name_len;

    for (size_t i = pwd_len; i < total_len; i++)
    {
        prompt.pwd[i] = dir_name[i - pwd_len];  
    }
    prompt.pwd[total_len] = '\0';
}