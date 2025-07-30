#include "file_system.h"

#include "fs_api.h"
#include "../mylibs/my_stdlib.h"
#include <stdbool.h>
#include "fs_structures.h"
#include "stdint.h"
#include "../mylibs/kernelio.h"

#define CLUSTER_SIZE 8 //in sectors

Disk_params disk_params;
FS_params fs_params = {0};
Working_dir working_dir = {0};

int set_cluster_status(uint32_t cluster_index, const  bool value);                    //forward dec
void set_record_name_plus_ext(record *rec, const char *name_plus_ext);
int clear_cluster(const uint32_t address_of_cluster);
void set_record_by_index(const record *rec_in, const uint32_t address_of_chain, const uint32_t index);
void get_record_by_index(record* rec_out, const uint32_t address_of_chain, const uint32_t index);
uint32_t get_address_of_cluster_in_chain(const uint32_t address_of_chain, const uint16_t index_of_address);
void set_address_of_cluster_in_chain(const uint32_t address_of_chain, const uint16_t index_of_address, const uint32_t value);
uint32_t get_free_cluster();

int fs_init()
{
    disk_init();
    
    fs_params.start_index = get_fs_start_index(); 

    disk_params.sector_count = get_sector_count();
 
    disk_params.cluster_count = disk_params.sector_count/8;

    //the table_size is aligned by sector
    fs_params.table_size = 1 + (disk_params.cluster_count/4096); //4096 bits (clusters) per sector

    //the start of data aligned by cluster, after table
    fs_params.data_zone_start = 1 + fs_params.start_index/8 + (fs_params.table_size + 1)/8; 

    BYTE buffer [512];    
    disk_read(buffer, fs_params.data_zone_start, 1);
    memcpy(&working_dir, buffer, sizeof(Working_dir));      //first 256 bytes abt the root dir in buffer

    return 0;
}

int mkfs()
{
    for (uint32_t i = fs_params.start_index; i < fs_params.data_zone_start; i++) {
        clear_cluster(i);
    }

    uint64_t magic = 0xC4A1C4A1C4A1C4A1;    //8B magic
    BYTE buffer[512] = {0};
    memcpy(buffer, &magic, sizeof(magic));
    disk_write(buffer, fs_params.start_index, 1);
    
    for (uint32_t i = 0; i < fs_params.data_zone_start; i++){
        set_cluster_status(i, 1);
    }   
    
    //---------------------------------
    //making root dir
    memset(&working_dir, 0, sizeof working_dir);
    memcpy(working_dir.rec.name, "root", 4);       // witout \0
    memcpy(working_dir.rec.extension, "dir", 3);   // without \0
    working_dir.rec.address_of_chain = fs_params.data_zone_start + 1;
    working_dir.rec.index_of_available_record = 1;

    clear_cluster(fs_params.data_zone_start);   //clearing everything in root dir 
    clear_cluster(fs_params.data_zone_start + 1);    //clearing chain

    set_cluster_status(fs_params.data_zone_start, 1);
    set_cluster_status(fs_params.data_zone_start + 1, 1);

    set_address_of_cluster_in_chain(working_dir.rec.address_of_chain, 0, fs_params.data_zone_start);
    set_record_by_index(&working_dir.rec, working_dir.rec.address_of_chain, 0);

    fs_params.is_initialized = true;

    return 0; //success
}

int detect_fs()
{
    BYTE buffer[512];
    
    disk_read(buffer, fs_params.start_index, 1);        
    
    //first 8 bytes of magic
    if (*((uint64_t*)buffer) == 0xC4A1C4A1C4A1C4A1)  
    {
        return 0; //success
    }
    
    return 1; //no fs
}

int makefile(const char* name_plus_ext)
{
    record rec = {0};
    set_record_name_plus_ext(&rec, name_plus_ext);

    rec.address_of_chain = get_free_cluster();
    set_cluster_status(rec.address_of_chain, 1);

    uint32_t start_cluster = get_free_cluster();
    set_address_of_cluster_in_chain(rec.address_of_chain, 0, start_cluster);
    set_cluster_status(start_cluster, 1);

    rec.size = 1; //size in clusters, 1 cluster for file

    set_record_by_index(&rec, working_dir.rec.address_of_chain, working_dir.rec.index_of_available_record);
    
    working_dir.rec.index_of_available_record++;
    
    if (strcmp(get_extension(&rec), "dir") == 0)
    {
        rec.index_of_available_record = 2;
        set_record_by_index(&rec, start_cluster, 0);
        set_record_by_index(&working_dir.rec, start_cluster, 1);
    }

    set_record_by_index(&working_dir.rec, working_dir.rec.address_of_chain, 0); //reload index_of_available_record

    return 0; //success
}

int clear_cluster(uint32_t address_of_cluster)
{
    BYTE zeros [512] = {0};
    uint32_t start_sector = (address_of_cluster) * CLUSTER_SIZE;

    for (int i = 0; i < CLUSTER_SIZE; i++){
        disk_write(zeros, start_sector + i, 1);
    }

    return 0;
}

int set_cluster_status(const uint32_t cluster_index, const bool value)
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

const uint32_t get_free_cluster()
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
                    free_cluster = (i-1)*4096 + B*8 + b;  
                    return free_cluster;
                }
            }
                
        }
        
    }    

    return 1; //error
}

void set_record_by_index(const record *rec_in, const uint32_t address_of_chain, const uint32_t index)
{
    uint32_t cluster_index = index/16;
    uint8_t rec_index_in_cluster = index%16;
    uint8_t sec_index = rec_index_in_cluster/2;

    uint32_t cluster_address = get_address_of_cluster_in_chain(address_of_chain, cluster_index);

    BYTE sec_with_record [512];
    disk_read(sec_with_record, (cluster_address*CLUSTER_SIZE) + sec_index, 1);

    uint8_t rec_pos_in_sector = rec_index_in_cluster%2;
    memcpy(sec_with_record + rec_pos_in_sector * sizeof(record), rec_in, sizeof(record));
    
    disk_write(sec_with_record, (cluster_address*CLUSTER_SIZE) + sec_index, 1);        
}

void get_record_by_index(record* rec_out, const uint32_t address_of_chain, const uint32_t index)
{
    uint32_t cluster_index = index/16; 
    uint8_t rec_index_in_cluster = index%16;
    uint8_t sec_index = rec_index_in_cluster/2;
    
    uint32_t cluster_address = get_address_of_cluster_in_chain(address_of_chain, cluster_index);

    BYTE sec_with_record[512];
    disk_read(sec_with_record, (cluster_address*CLUSTER_SIZE) + sec_index,1);

    uint8_t rec_pos_in_sector = rec_index_in_cluster%2;
    memcpy (rec_out, sec_with_record + rec_pos_in_sector * sizeof(record), sizeof(record));
}

const uint32_t get_address_of_cluster_in_chain(const uint32_t address_of_chain, const uint16_t index)
{
    uint16_t index_of_sector = index/128;
    uint16_t index_of_address_in_sector = index%128;

    uint32_t buffer [128];

    disk_read((BYTE*)buffer, (address_of_chain*CLUSTER_SIZE) + index_of_sector, 1);

    return buffer[index_of_address_in_sector];

}

void set_address_of_cluster_in_chain(const uint32_t address_of_chain, const uint16_t index_of_address, const uint32_t value)
{
    uint16_t index_of_sector = index_of_address/128;
    uint16_t index_of_address_in_sector = index_of_address%128;

    uint32_t buffer [128];

    disk_read((BYTE*)buffer, (address_of_chain*CLUSTER_SIZE) + index_of_sector, 1);


    buffer [index_of_address_in_sector] = value;

    disk_write((BYTE*)buffer, (address_of_chain*CLUSTER_SIZE) + index_of_sector, 1);
}

void set_record_name_plus_ext(record *rec, const char *name_plus_ext) {
    //finding a dot
    const char *dot = (const char *)strchr((const char*)name_plus_ext, '.');
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

const char *get_name_plus_ext(const record* rec)
{
    size_t name_len = 0;
    while (name_len < sizeof(rec->name) && rec->name[name_len] != 0)
        name_len++;

    size_t ext_len = 0;
    while (ext_len < sizeof(rec->extension) && rec->extension[ext_len] != 0)
        ext_len++;

    size_t name_ext_len = name_len + (ext_len ? 1 + ext_len : 0) + 1;

    static char buffer[48];

    if (name_ext_len > sizeof(buffer)) {
        buffer[0] = '\0';
        return (char*)buffer;
    }

    memcpy(buffer, rec->name, name_len);

    if (ext_len) {
        buffer[name_len] = '.';
        memcpy(buffer + name_len + 1, rec->extension, ext_len);
        buffer[name_ext_len - 1] = '\0';
    } else {
        buffer[name_len] = '\0';
    }

    return (char*)buffer;
}


const char *get_extension(const record *rec)
{
    static char extension[17]; //16 chars max and \0
    
    for (size_t i = 0; i < 17; i++)
    {
        if(rec->extension[i] == 0 || i == 17){
            extension[i] = '\0';
            break;
        }
        extension[i] = rec->extension[i];
    }
    
    return extension;
}

void copy_record(record* rec_out, const record* rec_in)
{
    memcpy(rec_out, rec_in, sizeof(record));
}

void list()     //SORRYYY, WILL BE FIXED SOON
{
    uint32_t i = 0;
    kprint_str("."); //this dir
    kprint_newline();
    i++;
    if(working_dir.rec.address_of_chain != fs_params.data_zone_start+1){ //predictable
        kprint_str("..");   //prev dir
        kprint_newline();
        i++;
    }

    for (; i < working_dir.rec.index_of_available_record; i++)
    {
        record rec_out = {0};
        get_record_by_index(&rec_out, working_dir.rec.address_of_chain, i);
        kprint_str(get_name_plus_ext(&rec_out));
        kprint_newline();
    }   
}

int change_dir(const char* dir_name)
{
    if (!dir_name) return 2;

    if(strcmp(dir_name, ".") == 0){
        return 0;   //lol
    }
    
    if(strcmp(dir_name, "..") == 0 && working_dir.rec.address_of_chain != fs_params.data_zone_start+1){
        copy_record(&working_dir.rec, &working_dir.rec_abt_prev);
        get_record_by_index(&working_dir.rec_abt_prev, working_dir.rec.address_of_chain, 1);    //index 1 is abt prev dir
        return 0;
    }

    record rec_buff;

    for(size_t i = 0; i < working_dir.rec.index_of_available_record; i++)
    {
        get_record_by_index(&rec_buff, working_dir.rec.address_of_chain, i);

        if(strcmp(get_name_plus_ext(&rec_buff), dir_name) == 0)
        {
            if (strcmp(get_extension(&rec_buff), "dir") != 0){
                return 2; // not dir!
            }
            copy_record(&working_dir.rec_abt_prev, &working_dir.rec);
            copy_record(&working_dir.rec, &rec_buff);
            return 0; //success
        }
    }

    return 1; //error, unknown dir
}

int write_in_file(const char* file_name, const BYTE* data, const uint32_t data_size, const uint32_t block_index_to_read)
{
    record rec_buff;
    for (size_t i = 0; i < working_dir.rec.index_of_available_record; i++)
    {
        get_record_by_index(&rec_buff, working_dir.rec.address_of_chain, i);
        if (strcmp(get_name_plus_ext(&rec_buff), file_name) == 0)
        {
            uint32_t cluster_to_read = get_address_of_cluster_in_chain(rec_buff.address_of_chain, block_index_to_read);
            clear_cluster(cluster_to_read);

            uint32_t written_bytes = 0;
            uint32_t current_sector = cluster_to_read * CLUSTER_SIZE;

            while (written_bytes < data_size)
            {
                uint32_t bytes_to_write = (data_size - written_bytes > 512) ? 512 : data_size - written_bytes;
                disk_write((BYTE*)(data + written_bytes), current_sector, 1);
                written_bytes += bytes_to_write;
                current_sector++;
            }
            return 0; //success
        }
    }
    return 1; //error, this file does not exist
}

char* read_from_file(const char* file_name, uint32_t* out_size, const uint32_t block_index_to_read)
{
    record rec_buff;
    set_record_name_plus_ext(&rec_buff, file_name);
    if(strcmp(get_extension(&rec_buff), "dir") == 0){
        return NULL; //error, its a directory, not a file
    }

    for (size_t i = 0; i < working_dir.rec.index_of_available_record; i++)
    {
        get_record_by_index(&rec_buff, working_dir.rec.address_of_chain, i);
        if (strcmp(get_name_plus_ext(&rec_buff), file_name) == 0)
        {
            uint32_t start_cluster = get_address_of_cluster_in_chain(rec_buff.address_of_chain, block_index_to_read);
            uint32_t size = rec_buff.size * CLUSTER_SIZE; // size in bytes
            char* buffer = kmalloc(size);
            if (!buffer) return NULL; // allocation failed

            uint32_t read_bytes = 0;
            uint32_t current_sector = start_cluster * CLUSTER_SIZE;

            while (read_bytes < size)
            {
                uint32_t bytes_to_read = (size - read_bytes > 512) ? 512 : size - read_bytes;
                disk_read((BYTE*)(buffer + read_bytes), current_sector, 1);
                read_bytes += bytes_to_read;
                current_sector++;
            }

            *out_size = size;
            return buffer; // success
        }
    }
    return NULL; // error, file not found
}