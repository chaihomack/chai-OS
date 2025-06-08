#pragma once

#include "../../mylibs/my_stdlib.h"

//ATA
BYTE ATA_disk_status();
BYTE ATA_disk_init();
int ATA_disk_read(BYTE* buff, uint32_t sector, uint32_t count);
int ATA_disk_write(const BYTE* buff, uint32_t sector, uint32_t count);
int ATA_disk_flush();
WORD ATA_get_word_from_DISK_IDENTIFY(uint32_t word_number);
unsigned int ATA_get_sector_count();