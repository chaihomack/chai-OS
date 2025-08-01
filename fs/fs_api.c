

#include "fs_api.h"
#include "../mylibs/my_stdlib.h"
#include "../drivers/disk_driver/disk_driver.h"
#include "../kernel/kernel.h"
#include "../memory_management/heap.h"

BYTE disk_init()
{
    	return ATA_disk_init();
}

int disk_read(BYTE* buff, uint32_t sector)
{
    	return ATA_disk_read(buff, sector, 1); //we dont use count cuz we are sigma 
}

int disk_write(const BYTE* buff, uint32_t sector)
{
    	return ATA_disk_write(buff, sector, 1);
}

uint32_t get_sector_count()
{
    	return ATA_get_sector_count();
}

void* calloc_api(uint16_t bytes_to_alloc)
{
	return kcalloc(bytes_to_alloc);
}