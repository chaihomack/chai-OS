#include "file_system.h"

#include "fs_api.h"
#include "../mylibs/my_stdlib.h"
#include <stdbool.h>
#include "fs_structures.h"
#include "stdint.h"
#include "../mylibs/kernelio.h"

#define CLUSTER_SIZE_IN_SECTORS 8 //in sectors
#define SECTOR_SIZE 512 //in bytes
#define CLUSTER_SIZE_IN_BYTES 4096

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
int32_t detect_fs();
void copy_record(record* rec_out, const record* rec_in);

void fs_params_init(uint32_t superblock_sector_index)
{
    fs_params.is_initialized = false;
    fs_params.start_sector_index = superblock_sector_index + 1;  // rc it is superblock sector index
    disk_params.sector_count = get_sector_count();
    
    disk_params.cluster_count = disk_params.sector_count/8;
    
    //the table_size is aligned by sector
    fs_params.table_size = (disk_params.cluster_count + 4095) / 4096; //4096 bits (clusters) per sector
    
    //the start of data aligned by cluster, after table
    fs_params.data_zone_start = 1 + (fs_params.start_sector_index + fs_params.table_size) / 8; //data zone start is cluster index
}

int fs_init()
{
    disk_init();
    int superblock_index = detect_fs();
    if (superblock_index == -1) {
        return -1; //error, no fs detected
    }
    fs_params_init(superblock_index);

    record rec_buff = {0};
    get_record_by_index(&rec_buff, fs_params.data_zone_start + 1, 0); // fs_params.data_zone_start + 1 its a address of root chain

    if(strcmp(get_name_plus_ext(&rec_buff), "root.dir") != 0){
        return -2;
    }
    copy_record(&working_dir.rec, &rec_buff);

    fs_params.is_initialized = true;
    return 0;
}

int mkfs(uint32_t superblock_sector_index)
{
    disk_init();
    fs_params_init(superblock_sector_index);

    for (uint32_t i = fs_params.start_sector_index/8; i < fs_params.data_zone_start; i++) {
        clear_cluster(i);
    }

    uint64_t magic = 0xC4A1C4A1C4A1C4A1;    //8B magic
    BYTE buffer[512] = {0};
    memcpy(buffer, &magic, sizeof(uint64_t));
    disk_write(buffer, superblock_sector_index);
    
    for (uint32_t i = 0; i < fs_params.data_zone_start; i++){
        set_cluster_status(i, 1);
    }
    
    //---------------------------------
    //making root dir
    memset(&working_dir, 0, sizeof(Working_dir));
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

// returns fs_start_index
int32_t detect_fs()
{   
    BYTE buffer[512] = {0};
    uint32_t sector_count = get_sector_count();
    for (uint32_t i = 0; i < sector_count; i++) {
        disk_read(buffer, i);
            if ((*((uint64_t*)buffer) == 0xC4A1C4A1C4A1C4A1)) {
            return i;   // fs detected
        }
    }
    return -1; //no fs
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
    uint32_t start_sector = (address_of_cluster) * CLUSTER_SIZE_IN_SECTORS;

    for (int i = 0; i < CLUSTER_SIZE_IN_SECTORS; i++){
        disk_write(zeros, start_sector + i);
    }

    return 0;
}

int is_bit_set(BYTE byte, int bit) {
    return (byte & (1 << bit)) ? 1 : 0;
}

BYTE toggle_bit(BYTE byte, int bit) {
    return byte ^ (1 << bit);
}

int set_cluster_status(const uint32_t cluster_index, const bool value)
{
    if(cluster_index >= disk_params.cluster_count || 
       (cluster_index < fs_params.data_zone_start && fs_params.is_initialized)) {
            return 1;
    }

    uint32_t byte_index = cluster_index / 8;
    uint32_t bit_index = cluster_index % 8;

    uint32_t sector_index = fs_params.start_sector_index + (byte_index / 512);
    uint32_t local_byte_index = byte_index % 512;

    BYTE buffer[512];
    disk_read(buffer, sector_index);

    if (is_bit_set(buffer[local_byte_index], bit_index) == value) {
        return 1; // already in desired state
    }

    if (value)
        buffer[local_byte_index] |= (1 << bit_index);
    else
        buffer[local_byte_index] &= ~(1 << bit_index);

    disk_write(buffer, sector_index);

    return 0;
}

uint32_t get_free_cluster()
{
    for (uint32_t i = fs_params.start_sector_index; i < fs_params.start_sector_index + fs_params.table_size; ++i)
    {
        BYTE buffer[512] = {0};
        disk_read(buffer, i);

        for (uint32_t B = 0; B < 512; ++B)
        {
            for (uint32_t b = 0; b < 8; ++b)
            {
                if(!is_bit_set(buffer[B], b)) {
                    return (i - fs_params.start_sector_index) * 4096 + B * 8 + b;
                }
            }   
        }
    }    
    return 0;
}

void set_record_by_index(const record *rec_in, const uint32_t address_of_chain, const uint32_t index)
{
    const uint32_t RECORDS_PER_CLUSTER = CLUSTER_SIZE_IN_BYTES / sizeof(record);

    uint32_t cluster_index = index / RECORDS_PER_CLUSTER;
    uint32_t record_in_cluster = index % RECORDS_PER_CLUSTER;
    
    uint32_t cluster_address = get_address_of_cluster_in_chain(address_of_chain, cluster_index);
    
    uint32_t sector_in_cluster = record_in_cluster / 2;
    uint32_t record_in_sector = record_in_cluster % 2;
    
    uint32_t sector = (cluster_address * CLUSTER_SIZE_IN_SECTORS) + sector_in_cluster;
    
    uint32_t offset = record_in_sector * sizeof(record);
    
    BYTE buffer[SECTOR_SIZE];
    disk_read(buffer, sector);
    memcpy(buffer + offset, rec_in, sizeof(record));
    disk_write(buffer, sector);
}

void get_record_by_index(record* rec_out, const uint32_t address_of_chain, const uint32_t index)
{
    const uint32_t RECORDS_PER_CLUSTER = CLUSTER_SIZE_IN_BYTES / sizeof(record);

    uint32_t cluster_index = index / RECORDS_PER_CLUSTER;
    uint32_t record_in_cluster = index % RECORDS_PER_CLUSTER;
    
    uint32_t cluster_address = get_address_of_cluster_in_chain(address_of_chain, cluster_index);
    
    uint32_t sector_in_cluster = record_in_cluster / 2;
    uint32_t record_in_sector = record_in_cluster % 2;
    
    uint32_t sector = (cluster_address * CLUSTER_SIZE_IN_SECTORS) + sector_in_cluster;
    
    uint32_t offset = record_in_sector * sizeof(record);

    BYTE buffer[SECTOR_SIZE];
    disk_read(buffer, sector);
    memcpy(rec_out, buffer + offset, sizeof(record));
}

const uint32_t get_address_of_cluster_in_chain(const uint32_t address_of_chain, const uint16_t index)
{
    uint16_t index_of_sector = index/128;
    uint16_t index_of_address_in_sector = index%128;

    uint32_t buffer [128];

    disk_read((BYTE*)buffer, (address_of_chain*CLUSTER_SIZE_IN_SECTORS) + index_of_sector);

    return buffer[index_of_address_in_sector];

}

void set_address_of_cluster_in_chain(const uint32_t address_of_chain, const uint16_t index_of_address, const uint32_t value)
{
    uint16_t index_of_sector = index_of_address/128;
    uint16_t index_of_address_in_sector = index_of_address%128;

    uint32_t buffer [128];

    disk_read((BYTE*)buffer, (address_of_chain*CLUSTER_SIZE_IN_SECTORS) + index_of_sector);


    buffer [index_of_address_in_sector] = value;

    disk_write((BYTE*)buffer, (address_of_chain*CLUSTER_SIZE_IN_SECTORS) + index_of_sector);
}

int add_new_cluster_to_chain(const uint32_t address_of_chain)
{
    uint32_t new_cluster = get_free_cluster();
    if (new_cluster == 1) {
        return 1; //error, no free clusters
    }

    set_cluster_status(new_cluster, 1);

    uint32_t last_index = 0;
    while (get_address_of_cluster_in_chain(address_of_chain, last_index) != 0) {
        last_index++;
    }

    set_address_of_cluster_in_chain(address_of_chain, last_index, new_cluster);

    return 0; //success
}

uint32_t get_cluster_count_in_chain(const uint32_t address_of_chain)
{
    uint32_t count = 0;
    uint32_t index = 0;

    while (get_address_of_cluster_in_chain(address_of_chain, index) != 0) {
        count++;
        index++;
    }

    return count;
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
    // uint32_t i = 0;
    // printf(".\n"); //this dir
    // i++;
    // if(working_dir.rec.address_of_chain != fs_params.data_zone_start+1){ //predictable
    //     printf("..\n");   //prev dir
    //     i++;
    // }

    // for (; i < working_dir.rec.index_of_available_record; i++)
    // {
    //     record rec_out = {0};
    //     get_record_by_index(&rec_out, working_dir.rec.address_of_chain, i);
    //     printf("%s\n", get_name_plus_ext(&rec_out));
    // }   
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

void write_data_in_cluster(const uint32_t cluster_index, BYTE* cluster_with_data)
{
    uint32_t sector_iterator = cluster_index * CLUSTER_SIZE_IN_SECTORS;
    
    for (size_t i = 0; i < CLUSTER_SIZE_IN_SECTORS; i++, sector_iterator++)
    {
        disk_write(cluster_with_data + i*512, sector_iterator);
    }
}

//data size in bytes
int write_in_file(const char* file_name, const BYTE* data, const uint32_t data_size)
{
    BYTE *cluster_with_data_buffer = calloc_api(CLUSTER_SIZE_IN_BYTES);
    record rec_buff;
    size_t data_size_in_clusters = (data_size + CLUSTER_SIZE_IN_BYTES - 1) / CLUSTER_SIZE_IN_BYTES;

    for (size_t i = 0; i < working_dir.rec.index_of_available_record; i++)
    {
        get_record_by_index(&rec_buff, working_dir.rec.address_of_chain, i);
        if (strcmp(get_name_plus_ext(&rec_buff), file_name) == 0)
        {
            size_t current_f_size = (size_t)get_cluster_count_in_chain(rec_buff.address_of_chain);
            while (current_f_size < data_size_in_clusters) {
                add_new_cluster_to_chain(rec_buff.address_of_chain);
                current_f_size = (size_t)get_cluster_count_in_chain(rec_buff.address_of_chain);
            }

            for (size_t i = 0; i < data_size_in_clusters-1; i++)
            {
                uint32_t cluster_iterator = get_address_of_cluster_in_chain(rec_buff.address_of_chain, i);
                memcpy(cluster_with_data_buffer, data + i*CLUSTER_SIZE_IN_BYTES, CLUSTER_SIZE_IN_BYTES);
                write_data_in_cluster(cluster_iterator, cluster_with_data_buffer);
                memset(cluster_with_data_buffer, 0, CLUSTER_SIZE_IN_BYTES);
            }
            size_t last_cluster_index = data_size_in_clusters - 1;
            size_t last_chunk_size = data_size % CLUSTER_SIZE_IN_BYTES;
            if (last_chunk_size == 0) last_chunk_size = CLUSTER_SIZE_IN_BYTES;

            memcpy(cluster_with_data_buffer, data + last_cluster_index * CLUSTER_SIZE_IN_BYTES, last_chunk_size);
            uint32_t end_cluster_index = get_address_of_cluster_in_chain(rec_buff.address_of_chain, last_cluster_index);
            write_data_in_cluster(end_cluster_index, cluster_with_data_buffer);

            // kfree(cluster_with_data_buffer);
            return 0; //success
        }
    }
    return 1; //error, this file does not exist
}

void read_data_from_cluster(uint32_t cluster_index, BYTE* out_buffer)
{
    uint32_t sector_iterator = cluster_index * CLUSTER_SIZE_IN_SECTORS;

    for (size_t i = 0; i < CLUSTER_SIZE_IN_SECTORS; i++, sector_iterator++)
    {
        disk_read(out_buffer + i * SECTOR_SIZE, sector_iterator);
    }
}

//output size is 4096 (1 cluster)
BYTE* read_from_file(const char* file_name, const uint32_t block_index_to_read)
{
    BYTE *cluster_with_data_buffer = calloc_api(CLUSTER_SIZE_IN_BYTES);

    record rec_buff;
    set_record_name_plus_ext(&rec_buff, file_name);
    if(strcmp(get_extension(&rec_buff), "dir") == 0){
        return NULL; // error, it's a directory
    }

    for (size_t i = 0; i < working_dir.rec.index_of_available_record; i++)
    {
        get_record_by_index(&rec_buff, working_dir.rec.address_of_chain, i);
        if (strcmp(get_name_plus_ext(&rec_buff), file_name) == 0)
        {
            uint32_t cluster_count = get_cluster_count_in_chain(rec_buff.address_of_chain);
            if (block_index_to_read >= cluster_count)
                return NULL; // error, out of bounds

            uint32_t target_cluster = get_address_of_cluster_in_chain(rec_buff.address_of_chain, block_index_to_read);

            read_data_from_cluster(target_cluster, (BYTE*)cluster_with_data_buffer);

            return cluster_with_data_buffer; // return the data from the cluster
        }
    }
    return NULL; // error, file not found
}