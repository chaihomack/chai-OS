#pragma once

#include "../mylibs/my_stdlib.h"
#include "stdint.h"
#include <stdbool.h>
#include "fs_structures.h"

int fs_init();
int detect_fs();

int mkfs();

const char *get_name_plus_ext(const record* rec);
const char *get_extension(const record *rec);

int makefile(const char* name_plus_ext);
void list();
int change_dir(const char* dir_name);

extern Working_dir working_dir;
