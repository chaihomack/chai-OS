
#include "shell.h"
#include "../drivers/keyboard/keyboard_driver.h"
#include "../mylibs/my_stdlib.h"
#include "../mylibs/kernelio.h"

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
        char command[size_of_command];
        memset(command, 0, size_of_command);

        kprint_str(prompt.pwd);
        kprint_str(prompt.text_af_pwd);
        
        kscanf(command, size_of_command);

        kprint_newline();
        do_command(command);
    }
       
}

void add_dir_in_prompt(const char* dir_name)
{
    size_t pwd_len = strlen(prompt.pwd);
    size_t name_len = strlen(dir_name);
    size_t total_len = pwd_len + name_len;

    for (size_t i = pwd_len; i < total_len; i++)
    {
        prompt.pwd[i] = dir_name[i - pwd_len];  
    }
    prompt.pwd[total_len] = '/';
    prompt.pwd[total_len+1] = '\0';
}

void increment_dir_in_prompt()
{
    size_t pwd_len = strlen(prompt.pwd);

    prompt.pwd[pwd_len--] = 0;          //rming \0
    prompt.pwd[pwd_len--] = 0;          //rming '/', its last char
    for (size_t i = pwd_len; i >= 0; i--)
    {
        if(prompt.pwd[i] == '/'){
            return;
        }
        prompt.pwd[i] = 0;
        pwd_len--;
    }

    prompt.pwd[pwd_len] = '\0';
}
