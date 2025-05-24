#include "commands.h"

#include "string.h"

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
void help();
int parse_args(const char* input, char args[MAX_ARGS][MAX_ARG_LEN]);   //forward declaration
int strcmp(const char* s1, const char* s2); 

struct command_list commands[] = {
    { "help", help },
    { "clear", clear },
    { NULL, NULL }  
};

void call_command(const char* input) {
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(input, commands[i].name) == 0) {
            commands[i].func();
            return;
        }
    }

    kprint("Unknown command");
}

void do_command(const char* input_command)
{
    command cmd;
    
    parse_args(input_command, cmd.parameters);

    call_command(cmd.parameters[0]);             // call command using first parameter
}


int parse_args(const char* input_command, char args[MAX_ARGS][MAX_ARG_LEN]) {
    int arg_i = 0;     // whitch argument rn
    int char_i = 0;    // position in argument

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

void help()
{

}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {       s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}