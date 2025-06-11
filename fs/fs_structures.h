#include "stdint.h"
#include "mylibs/my_stdlib.h"

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

typedef struct 
{
    uint32_t path[512];
    uint32_t address;
} Working_dir;

typedef struct __attribute__((__packed__)) {
    BYTE   name[32];                    // without '\0'
    BYTE   extension[16];               // without '\0'
    uint32_t address;
    uint32_t count_of_existing_dirs;    //only for dirs
    BYTE   additional_data_for_future[200];
} record;