
#include "../mylibs/my_stdlib.h"




BYTE disk_status();
BYTE disk_init();
int disk_read(BYTE* buff, DWORD sector, DWORD count);
int disk_write(const BYTE* buff, DWORD sector, DWORD count);
int disk_flush();
WORD get_word_from_DISK_IDENTIFY(uint32_t word_number);
uint32_t get_sector_count();