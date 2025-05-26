#pragma once

#include "../../mylibs/my_stdlib.h"

//ATA
BYTE ATA_disk_status();
BYTE ATA_disk_init();
int ATA_disk_read(BYTE* buff, DWORD sector, DWORD count);
int ATA_disk_write(const BYTE* buff, DWORD sector, DWORD count);
int ATA_disk_flush();
WORD ATA_get_word_from_DISK_IDENTIFY(unsigned int word_number);
unsigned int ATA_get_sector_count();