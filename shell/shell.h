#pragma once

#include "../mylibs/my_stdlib.h"
#include "commands/commands.h"

typedef struct{
    char pwd[512];
    char text_af_pwd[8];
}Prompt;

void start_shell();

void add_dir_in_prompt(const char* dir_name);
void increment_dir_in_prompt();