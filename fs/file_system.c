#include "file_system.h"

#include "diskio.h"
#include "string.h"
#include "../mylibs/my_stdlib.h"
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

typedef struct 
{
    uint32_t pwd[512];
} Data_for_working;

Data_for_working data_for_working = 
{
    .pwd = {0}
};

uint32_t set_cluster_status(uint32_t cluster_index, bool value); //forward dec

int fs_init()
{
    disk_init();

    disk_params.sector_count = get_sector_count();
    disk_params.cluster_count = disk_params.sector_count/8;

    //the table_size is aligned by sector
    fs_params.table_size = disk_params.cluster_count/4096; //4096 bits (clusters) per sector

    //the start of data aligned by cluster, after table
    fs_params.data_zone_start = 1 + (fs_params.table_size + 1)/8; 

    return 0;
}

int mkfs()
{
    BYTE zeros[512] = {0};
    for (int i = 1; i < fs_params.table_size+8; i++){ //8 for reserved cluster
        disk_write(zeros, i, 1);    //clearing the table
    }

    uint64_t magic = 0xC4A1C4A1C4A1C4A1;    //8B magic
    disk_write((BYTE*)&magic, 0, 1);  //will be fixed when I make a bootloader
    
    //<= cuz root dir
    for (uint32_t i = 0; i <= fs_params.data_zone_start; i++){
        set_cluster_status(i, 1);
    }

    fs_params.is_initialized = true;

    return 0; //success
}

int detect_fs()
{
    BYTE buffer[512];
    
    //magic number is on sector 0 //will be fixed when I make a bootloader!!!
    disk_read(buffer, 0, 1);        
    
    //first 8 bytes of magic
    if (*((uint64_t*)buffer) == 0xC4A1C4A1C4A1C4A1)  
    {
        return 0; //success
    }
    
    return 1; //no fs
}

uint32_t set_cluster_status(uint32_t cluster_index, bool value)
{
    if(cluster_index > disk_params.cluster_count || 
        (cluster_index <= fs_params.data_zone_start 
            && fs_params.is_initialized == true)){ 
                return 1; //error
    }

    uint32_t byte_index = cluster_index / 8;
    uint8_t bit_index = cluster_index % 8;
    uint32_t sector_index = 1 + (byte_index / 512); //32 bits will be fine :3
    
    BYTE buffer[512];

    disk_read(buffer, sector_index, 1);

    //everything should be predictable
    if(is_bit_set(buffer[byte_index], bit_index) == value){     
        return 1; //error
    }

    buffer[byte_index] = toggle_bit(buffer[byte_index], bit_index); 

    disk_write(buffer, sector_index, 1);

    return 0;
}

uint32_t get_free_cluster()
{
    for (uint32_t i = 1; i < fs_params.table_size; i++)
    {
        BYTE buffer [512];
        disk_read(buffer, i, 1);

        for (uint32_t B = 0; B < 512; B++)
        {
            for (uint32_t b = 0; b < 8; b++)
            {
                if(is_bit_set(buffer[B], b) == 0){
                    return B*8 + b;
                }
            }
                
        }
        
    }    
    return 1; //error
}


