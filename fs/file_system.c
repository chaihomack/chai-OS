#include "file_system.h"

#include "diskio.h"
#include "string.h"
#include "../mylibs/my_stdlib.h"
#include <stdbool.h>
#include "fs_structures.h"
#include "stdint.h"

#define CLUSTER_SIZE 8 //in sectors

Disk_params disk_params;
FS_params fs_params = {.is_initialized = 0};
Working_dir working_dir = {0};

uint32_t set_cluster_status(uint32_t cluster_index, bool value);                    //forward dec
void set_record_name_plus_ext(record *rec, const uchar *name_plus_ext);
int clear_cluster(uint32_t* adress_of_cluster);

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

    record root =
    {
        .name = {'r', 'o', 'o', 't'},       //not "root" cuz \0
        .extension = "dir",
        .address = fs_params.data_zone_start,
        .count_of_existing_dirs = 0,
        .additional_data_for_future = {0}
    };

    BYTE buff[512] = {0}; 
    for (size_t i = 0; i < 8; i++)
    {
        disk_write(buff, (root.address*8) + i, 1);  //clearing everything in root dir
    } 
    
    memcpy(buff, &root, sizeof(record));        //firs 256 bytes its about this dir
    
    disk_write(buff, fs_params.data_zone_start*8, 1);
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

int mkfile(const uchar* name_plus_ext)
{
    uint32_t start_cluster = get_free_cluster;
    
    BYTE *buffer [512];
    disk_read(buffer, working_dir.address*8, 1);
            

    record rec = {0};
    set_record_name_plus_ext(&rec, name_plus_ext);


}

int clear_cluster(uint32_t* address_of_cluster)
{
    BYTE zeros [512] = {0};
    uint32_t start_sector = (*address_of_cluster) * CLUSTER_SIZE;

    for (int i = 0; i < CLUSTER_SIZE; i++)
    {
        disk_write(zeros, start_sector + i, 1);
    }

    return 0;
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
    uint32_t free_cluster = 1;
    for (uint32_t i = 1; i < fs_params.table_size; ++i)
    {
        BYTE buffer [512];
        disk_read(buffer, i, 1);

        for (uint32_t B = 0; B < 512; ++B)
        {
            for (uint32_t b = 0; b < 8; ++b)
            {
                if(is_bit_set(buffer[B], b) == 0){
                    free_cluster = B*8 + b;    
                }
            }
                
        }
        
    }    

    return 1; //error
}

void set_record_name_plus_ext(record *rec, const uchar *name_plus_ext) {
    //finding a dot
    const uchar *dot = (const uchar *)strchr((const char*)name_plus_ext, '.');
    size_t name_len = dot ? (size_t)(dot - name_plus_ext) : strlen((const char*)name_plus_ext);
    if (name_len > sizeof rec->name) 
        name_len = sizeof rec->name;

    // clearing the arrays
    memset(rec->name, 0, sizeof rec->name);
    memset(rec->extension, 0, sizeof rec->extension);

    // copying thename
    memcpy(rec->name, name_plus_ext, name_len);

    // if extension exist, copy
    if (dot) {
        const unsigned char *ext = dot + 1;
        size_t ext_len = strlen((const char*)ext);
        if (ext_len > sizeof rec->extension)
            ext_len = sizeof rec->extension;
        memcpy(rec->extension, ext, ext_len);
    }
}