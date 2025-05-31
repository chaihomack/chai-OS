#pragma once
#include "../mylibs/my_stdlib.h"
#include "stdint.h"
#include <stdbool.h>

typedef struct 
{
    uint32_t sector_count, cluster_count;
}Disk_params;

Disk_params disk_params;

typedef struct
{   
    uint32_t table_size;            //the table_size is aligned by sector
    uint32_t data_zone_start;       //start of data aligned by cluster, after table
    bool is_initialized;
}FS_params;

FS_params fs_params = {.is_initialized = 0};

int mkfs();
int detect_fs();

uint32_t get_free_cluster();