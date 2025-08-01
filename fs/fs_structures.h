#pragma once

#include "stdint.h"
#include "../mylibs/my_stdlib.h"

typedef struct 
{
    uint32_t sector_count, cluster_count;
}Disk_params;

typedef struct
{   
    uint32_t start_sector_index; 
    uint32_t table_size;            //the table_size is aligned by sector and it is in sectors
    uint32_t data_zone_start;       //start of data aligned by cluster, after table and it is a cluster index
    bool is_initialized;
}FS_params;

typedef struct __attribute__((__packed__)) {
    char   name[32];                    // without '\0'
    char extension[16];               // without '\0'
    uint32_t address_of_chain;
    uint32_t index_of_available_record;    //for dirs
    uint32_t size;                         //in clusters
    BYTE additional_data_for_future[196];
} record;

typedef struct 
{
    record rec;
    record rec_abt_prev;
} Working_dir;

typedef struct __attribute__((__packed__)){
    uint32_t cluster_with_data [1024];
} chain_with_more_data;
