#include "diskio.h"
#include "file_system.h"
#include "string.h"


int mkfs()
{
    disk_params.sector_count = get_sector_count();
    disk_params.cluster_count = disk_params.sector_count/8;
    
    fs_params.table_size = 1 + disk_params.cluster_count/512; //the table_size is aligned by sector
    fs_params.data_zone_start = 1 + (fs_params.table_size + 1)/8; //the start of data aligned by cluster, after table

    BYTE fs_info_on_disk[16] = {0xC4, 0xA1, 0xC4, 0xA1, 0xC4, 0xA1, 0xC4, 0xA1,     //8B magic
    fs_params.data_zone_start,                                                      //minimum available address
    fs_params.data_zone_start};                                                     //4B for minimum available address

    disk_write(fs_info_on_disk, 1, 1);
}

int detect_fs()
{
    BYTE buffer[512];
    disk_read(buffer, 1, 1);        //magic number is on 2 sector 
    
    if (*((uint64_t*)buffer) == 0xC4A1C4A1C4A1C4A1)  //first 8 bytes of magic
    {
        return 0; //success
    }
    
    return 1; //no fs
}

int make_file(char* input_name, char* input_extension, uint32_t file_size_in_KiB)
{
    if (strlen(input_name) > 8)           //only 8 bytes so only 8 characters!
    {
        return 1;
    }
    if(strlen(input_extension) > 3)       //only 3 bytes so only 3 characters!
    {
        return 1;
    }

    file_allocation_table_record record =
    {
        .name = input_name,
        .extension = input_extension,
        .size = 1 + (file_size_in_KiB / 4),      //aligned by cluster (cluster = 4KiB)
        .adress
    };
}