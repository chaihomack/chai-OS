#include "commands.h"

#include "../../mylibs/my_stdlib.h"
#include "../../fs/file_system.h"
#include "../shell.h"

#define MAX_ARGS 10
#define MAX_ARG_LEN 64

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef struct{

    char parameters[MAX_ARGS][MAX_ARG_LEN];

}command;

struct command_list {
    const char* name;
    void (*func)();
};

void clear();
void test();
void go_fs();

int parse_args(const char* input, char args[MAX_ARGS][MAX_ARG_LEN]);   //forward declaration

struct command_list commands[] = {
    { "test", test },
    { "clear", clear },
    { "gofs", go_fs },
    { NULL, NULL }  
};

void call_command(const char* input) {
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(input, commands[i].name) == 0) {
            commands[i].func();
            return;
        }
    }

    kprint_str("Unknown command");
}

void do_command(const char* input_command)
{
    command cmd;
    
    parse_args(input_command, cmd.parameters);

    call_command(cmd.parameters[0]);             // call command using first parameter
}


int parse_args(const char* input_command, char args[MAX_ARGS][MAX_ARG_LEN]) {
    int arg_i = 0;     // which argument rn
    int char_i = 0;    // position iconnect_to_fs argument

    for (int i = 0; ; ++i) {
        char c = input_command[i];

        // the end of the line
        if (c == '\0') {
            if (char_i > 0 && arg_i < MAX_ARGS) {
                args[arg_i][char_i] = '\0';
                arg_i++;
            }
            break;
        }

        // argument separation
        if (c == ' ' || c == '\t') {
            if (char_i > 0 && arg_i < MAX_ARGS) {
                args[arg_i][char_i] = '\0';
                arg_i++;
                char_i = 0;
            }
        } else {
            if (arg_i < MAX_ARGS && char_i < MAX_ARG_LEN - 1) {
                args[arg_i][char_i++] = c;
            }
        }
    }

    return arg_i; // number of arguments
}

extern struct Cursor cursor;

void clear()
{
    clear_screen();
    cursor.loc = 0;
}

void test()
{
    kprint_str(get_extension(&working_dir.rec));
}

void go_fs()        //tmp command, will be removed soon
{
    fs_init();

    if (detect_fs())
    {
        if(mkfs())
        {
            kprint_str("error while mkfs");
            kgetc();
        }
        kprint_str("mkfs success");
    }else{
        kprint_str("fs is found");

    }
    
    add_dir_in_prompt(get_name_plus_ext(&working_dir.rec));
}        