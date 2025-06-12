#include "file_system.h"

#include "diskio.h"
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
int clear_cluster(uint32_t adress_of_cluster);
void set_record_in_cluster(record *rec, uint32_t index_of_cluster_for_file, uint8_t index_of_sector_with_record, uint8_t index_of_record_in_cluster);
uint32_t get_adress_of_cluster_in_chain(uint32_t cluster_with_chain, uint16_t index_of_adress);
void set_adress_of_cluster_in_chain(uint32_t cluster_with_chain, uint16_t index_of_adress, uint32_t value);
uint32_t get_free_cluster();
int mkdir (record *rec, uint32_t adress_of_chain_start);

int fs_init()
{
    disk_init();
 
    disk_params.sector_count = get_sector_count();
    disk_params.cluster_count = disk_params.sector_count/8;

    //the table_size is aligned by sector
    fs_params.table_size = disk_params.cluster_count/4096; //4096 bits (clusters) per sector

    //the start of data aligned by cluster, after table
    fs_params.data_zone_start = 1 + (fs_params.table_size + 1)/8; 

    BYTE buffer [512];    
    disk_read(buffer, fs_params.data_zone_start, 1);
    memcpy(&working_dir, buffer, sizeof(Working_dir));      //first 256 bytes abt the root dir in buffer


    return 0;
}

int mkfs()
{
    for (uint32_t i = 1; i < fs_params.data_zone_start; i++)
    {
        clear_cluster(i);
    }
    
    uint64_t magic = 0xC4A1C4A1C4A1C4A1;    //8B magic
    disk_write((BYTE*)&magic, 0, 1);  //will be fixed when I make a bootloader
    
    for (uint32_t i = 0; i < fs_params.data_zone_start; i++){
        set_cluster_status(i, 1);
    }

    //---------------------------------
    //making root dir
    record root =
    {
        .name = {'r', 'o', 'o', 't'},       //not "root" cuz \0
        .extension = {'d', 'i', 'r'},
        .adress_of_chain = fs_params.data_zone_start + 1,          //just after table, cuz its first files, its predictable
        .adress_of_available_record = 1,
        .additional_data_for_future = {0}
    };

    clear_cluster(fs_params.data_zone_start);   //clearing everything in root dir 

    BYTE buff [512] = {0};
    memcpy(buff, &root, sizeof(record));        //first 256 bytes its about this dir
    
    set_record_in_cluster(&root, fs_params.data_zone_start, 0, 0);
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
    BYTE buffer [512];
    disk_read(buffer, working_dir.address*8, 1);
    record this_dir_info;
    memcpy(&this_dir_info, buffer, sizeof(record));

    uint32_t index_of_cluster_for_file = (this_dir_info.adress_of_available_record + 1)/16;      //+1 cuz its index
    uint8_t index_of_record_in_cluster = (this_dir_info.adress_of_available_record+1)%16;
    uint8_t index_of_sector_with_record = (index_of_record_in_cluster+1)/2;

    record rec = {0};
    set_record_name_plus_ext(&rec, name_plus_ext);

    uint32_t free_cluster = get_free_cluster();
    rec.adress_of_chain = free_cluster;
    set_cluster_status(free_cluster, 1);

    uint32_t start_cluster = get_free_cluster();
    set_adress_of_cluster_in_chain(rec.adress_of_chain, 0, start_cluster);

    mkdir(&rec, start_cluster);

    return 0; //success
}

int mkdir (record *rec, uint32_t adress_of_chain_start)
{
    set_record_in_cluster(rec, adress_of_chain_start, 0, 0);
    
}


int clear_cluster(uint32_t address_of_cluster)
{
    BYTE zeros [512] = {0};
    uint32_t start_sector = (address_of_cluster) * CLUSTER_SIZE;

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
//imlosingmyself

void set_record_in_cluster(record *rec, uint32_t index_of_cluster_for_file, uint8_t index_of_sector_with_record, uint8_t index_of_record_in_cluster)
{
    BYTE sec_with_record [512];
    if((index_of_record_in_cluster+1)%2 == 1)
    {        
        memcpy(sec_with_record, rec, sizeof(record));          //first 256 bytes of sector
    }else{      // ...%2 == 0
        memcpy(sec_with_record + 256, rec, sizeof(record));    //second 256 bytes of sector
    }

    if(index_of_cluster_for_file == 0)
    {
        disk_write(sec_with_record, index_of_cluster_for_file*CLUSTER_SIZE + index_of_sector_with_record, 1);
    }           
}

uint32_t get_adress_of_cluster_in_chain(uint32_t cluster_with_chain, uint16_t index_of_adress)
{
    uint16_t index_of_sector = index_of_adress/128;
    uint16_t index_of_adress_in_sector = index_of_adress%128;

    uint32_t buffer [128];

    disk_read((BYTE*)buffer, cluster_with_chain + index_of_sector, 1);


    return buffer[index_of_adress_in_sector];

}

void set_adress_of_cluster_in_chain(uint32_t cluster_with_chain, uint16_t index_of_adress, uint32_t value)
{
    uint16_t index_of_sector = index_of_adress/128;
    uint16_t index_of_adress_in_sector = index_of_adress%128;

    uint32_t buffer [128];

    disk_read((BYTE*)buffer, cluster_with_chain + index_of_sector, 1);


    buffer [index_of_adress_in_sector] = value;

    disk_read((BYTE*)buffer, cluster_with_chain + index_of_sector, 1);
}

void set_record_name_plus_ext(record *rec, const uchar *name_plus_ext) {
    //finding a dot
    const uchar *dot = (const uchar *)strchr((const uchar*)name_plus_ext, '.');
    size_t name_len = dot ? (size_t)(dot - name_plus_ext) : strlen((const char*)name_plus_ext);
    if (name_len > sizeof rec->name) 
        name_len = sizeof rec->name;

    // clearing the arrays
    memset(rec->name, 0, sizeof rec->name);
    memset(rec->extension, 0, sizeof rec->extension);

    // copying the name
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

uchar *get_name_plus_ext(record* rec)
{
    size_t name_len = 0;

    while(sizeof rec->name && rec->name[name_len] != 0){            //to fist 0 ,,,, no null terminators :)
        name_len++;
    }

    size_t ext_len = 0;

    while(sizeof rec->extension && rec->extension[ext_len] != 0){   //to fist 0 ,,,, no null terminators :)
        ext_len++;
    }

    size_t name_ext_len = name_len + (ext_len ? 1 + ext_len : 0) + 1;

    static uchar buffer [48];      //32 for name and 16 for extension    
    
    if(name_ext_len > sizeof buffer)
    {
        buffer[0] = '\0';    //hueta
        return buffer;
    }

    memcpy(buffer, rec->name, name_len);


    if(ext_len)         
    {
        buffer[name_len] = '.';
        memcpy(buffer + name_len + 1, rec->extension, ext_len);
        buffer[name_ext_len - 1] = '\0';
    }else{
        buffer[name_len] = '\0';
    }

    return buffer;
}