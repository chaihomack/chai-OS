#pragma once

#include "../mylibs/my_stdlib.h"
#include "commands/commands.h"

typedef struct{
    uchar pwd[512];
    uchar text_af_pwd[8];
}Prompt;

void start_shell();
void add_dir_in_prompt(const uchar* dir_name);
