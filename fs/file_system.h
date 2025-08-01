#pragma once

#include "../mylibs/my_stdtypes.h"
#include "stdint.h"
#include <stdbool.h>
#include "fs_structures.h"
#include <string.h>

int fs_init();
int32_t detect_fs();

int mkfs(uint32_t fs_start_index);

const char *get_name_plus_ext(const record* rec);
const char *get_extension(const record *rec);

int makefile(const char* name_plus_ext);
void list();                                    //SORRYYY, WILL BE FIXED SOON
int change_dir(const char* dir_name);

int write_in_file(const char* file_name, const BYTE* data, const uint32_t data_size);
BYTE* read_from_file(const char* file_name, const uint32_t block_index_to_read);