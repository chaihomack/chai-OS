#pragma once
#include "../mylibs/my_stdlib.h"
#include "stdint.h"

typedef struct __attribute__((__packed__))      //no aligtment for bytes
{
    unsigned char r_name[8];                      // 8 characters for name
    unsigned char r_extension[3];            // 3 characters for fileextension 
    uint8_t r_size;                               // 1 byte for size (255 clusters max - 1020 KiB)
    uint32_t r_adress;                            // 4 bytes for adressing(in clusters 268,435,456 clusters (16 TB) maximum)    
}file_allocation_table_record;

typedef struct 
{
    uint32_t sector_count, cluster_count;
}Disk_params;

Disk_params disk_params;

typedef struct
{   
    uint32_t table_size;            //the table_size is aligned by sector
    uint32_t data_zone_start;       //start of data aligned by cluster, after table
    uint32_t record_count;           //4 Bytes after magic in 2 sector
    uint32_t minimum_available_adress;
}FS_params;

FS_params fs_params;

int mkfs();
int detect_fs();