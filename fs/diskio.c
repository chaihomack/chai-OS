

#include "diskio.h"
#include "../mylibs/my_stdlib.h"
#include "../drivers/disk_driver/disk_driver.h"


BYTE disk_status()
{
    return ATA_disk_status();
}

BYTE disk_init()
{
    return ATA_disk_init();
}

int disk_read(BYTE* buff, DWORD sector, DWORD count)
{
    return ATA_disk_read(buff, sector, count);
}

int disk_write(const BYTE* buff, DWORD sector, DWORD count)
{
    return ATA_disk_write(buff, sector, count);
}

int disk_flush()
{
    return ATA_disk_flush();
}

WORD get_word_from_DISK_IDENTIFY(uint32_t word_number)
{
    return ATA_get_word_from_DISK_IDENTIFY(word_number);
}

uint32_t get_sector_count()
{
    return ATA_get_sector_count();
}