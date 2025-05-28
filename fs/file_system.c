#include "diskio.h"
#include "file_system.h"
#include "string.h"
#include "../mylibs/my_stdlib.h"

int mkfs()
{
    disk_params.sector_count = get_sector_count();
    disk_params.cluster_count = disk_params.sector_count/8;
    
    fs_params.table_size = 2 + disk_params.cluster_count/512; //the table_size is aligned by sector //1 reserved sector  cuz first 16B in 2st sector
    fs_params.data_zone_start = 1 + (fs_params.table_size + 1)/8; //the start of data aligned by cluster, after table

    BYTE fs_info_on_disk[16] = {0xC4, 0xA1, 0xC4, 0xA1, 0xC4, 0xA1, 0xC4, 0xA1,     //8B magic
    1,                                                      //record count (1 for first 16 bits)
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

int split_into_name_and_extension(uchar *name_with_extension, uchar* name, uchar* extension)
{
    if(strlen(name_with_extension) > 12){ //8 chars of name + dot + 3 for extension maximum
        return 1;
    }
    
    size_t i;
    for (i = 0; i < 8; ++i) // 8 chars
    {
        if (name_with_extension[i] == '.' || name_with_extension[i] == '\0')
        {
            break;
        }
        
        name[i] = name_with_extension[i];
    }

    if(name_with_extension[i] != '.'){
        return 0;
    }
    
    i++; //index after a dot
    for (size_t j = 0; j < 3; ++j, ++i)
    {
        if(name_with_extension[i] == '\0'){
            return 0;
        }
        extension[j] = name_with_extension[i];      //3 chars after dot
    }
    
    return 0;
}

int make_file(uchar* name_with_extension, uint32_t file_size_in_KiB)
{
    uchar name [8] = {' '};
    uchar extension [3] = {' '};          //setting empty fields for correct split_into_name_and_extension 

    if (split_into_name_and_extension(name_with_extension, name, extension)){
        return 1;
    }

    file_allocation_table_record record =
    {
        .r_name = name,
        .r_extension = extension,
        .r_size = 1 + (file_size_in_KiB / 4),
        .r_adress = fs_params.minimum_available_adress
    };


    file_allocation_table_record buffer[32];        //32 * 16 = 512         //1 record = 16 bytes

    uint32_t which_sector_to_change = fs_params.data_zone_start + (fs_params.record_count / 32);    //32 records per sector
    disk_read(buffer, which_sector_to_change, 1);

    buffer[fs_params.record_count % 32] = record;

    disk_write(buffer, which_sector_to_change, 1);
}

