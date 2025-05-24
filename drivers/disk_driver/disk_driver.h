#pragma once

#include "../../mylibs/my_stdlib.h"

//ATA
BYTE ATA_disk_status();
BYTE ATA_disk_initialize();
int ATA_disk_read(BYTE* buff, DWORD sector, DWORD count);
int ATA_disk_write(const BYTE* buff, DWORD sector, DWORD count);
int ATA_disk_flush();