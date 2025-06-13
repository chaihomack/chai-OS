#pragma once

#include "../mylibs/my_stdlib.h"
#include "stdint.h"
#include <stdbool.h>
#include "fs_structures.h"

int fs_init();
int detect_fs();

int mkfs();
int mkfile(const char* name_plus_ext);

char *get_name_plus_ext(record* rec);
char *get_extension(record *rec);

extern Working_dir working_dir;
