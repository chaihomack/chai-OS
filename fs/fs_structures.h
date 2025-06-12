#pragma once

#include "stdint.h"
#include "../mylibs/my_stdlib.h"

typedef struct 
{
    uint32_t sector_count, cluster_count;
}Disk_params;

typedef struct
{   
    uint32_t table_size;            //the table_size is aligned by sector
    uint32_t data_zone_start;       //start of data aligned by cluster, after table
    bool is_initialized;
}FS_params;

typedef struct __attribute__((__packed__)) {
    BYTE   name[32];                    // without '\0'
    BYTE   extension[16];               // without '\0'
    uint32_t adress_of_chain;
    uint32_t adress_of_available_record;    //for dirs
    BYTE   additional_data_for_future[200];
} record;

typedef struct 
{
    uint32_t address;
    record rec;
} Working_dir;

typedef struct __attribute__((__packed__)){
    uint32_t cluster_with_data [1024];
} chain_with_more_data;
