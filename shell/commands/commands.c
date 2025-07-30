#include "commands.h"

#include "../../mylibs/my_stdlib.h"
#include "../../fs/file_system.h"
#include "../shell.h"
#include "../../mylibs/kernelio.h"
#include "../textr/textr.h"

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
void go_fs();
void mkfile();
void cd();
void echo();
void ReadFile();
void WriteInFile();
void textr();
int parse_args(const char* input, char args[MAX_ARGS][MAX_ARG_LEN]);   //forward declaration
void clear_params();

struct command_list commands[] = {
    { "readfile", ReadFile },
    { "writeinfile", WriteInFile },
    { "echo", echo},
    { "ls", list },
    { "clear", clear },
    { "gofs", go_fs },
    { "mkfile", mkfile },
    { "cd", cd},
    { "textr", textr},
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

command cmd = {0}; //sorry

void do_command(const char* input_command)
{   
    parse_args(input_command, cmd.parameters);

    call_command(cmd.parameters[0]);             // call command using first parameter

    clear_params();
}

void clear_params()
{
    for (size_t i = 0; i < MAX_ARGS; i++)
    {
        for (size_t j = 0; j < MAX_ARG_LEN; j++)
        {
            cmd.parameters[i][j] = 0;
        }
        
    }
    
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
    set_cursor_pos(0, 0);
}

extern Working_dir working_dir;

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

void mkfile()
{
    makefile(cmd.parameters[1]);
}

void cd()
{
        int res = change_dir(cmd.parameters[1]); 
        if(res == 1){
                kprint_str("unknown dir");
                return;
        }
        if(res == 2){
                kprint_str("not a dir");
                return;
        }
        if (strcmp(cmd.parameters[1], "..") == 0){
                increment_dir_in_prompt();
                return;
        }

        if(strcmp(cmd.parameters[1], ".") != 0){
                add_dir_in_prompt(get_name_plus_ext(&working_dir.rec));
        }
}

void echo()
{
    kprint_str(cmd.parameters[1]);
}

void WriteInFile()
{
    write_in_file(cmd.parameters[1], cmd.parameters[2], 512, 0); // just for testing
    kprint_str("Data written to file");
}

void ReadFile()
{
    uint32_t size;
    char* data = read_from_file(cmd.parameters[1], &size, 0);   // just for testing
    if (data) {
        kprint_str(data);
        kprint_newline();
    } else {
        kprint_str("Error reading file");
    }
}

void textr()
{
    textr_start(cmd.parameters[1]);
}