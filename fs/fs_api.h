
#include "../mylibs/my_stdlib.h"

#include "../kernel/kernel.h"


BYTE disk_init();
int disk_read(BYTE* buff, uint32_t sector);
int disk_write(const BYTE* buff, uint32_t sector);
WORD get_word_from_DISK_IDENTIFY(uint32_t word_number);
uint32_t get_sector_count();
void* calloc_api(uint16_t bytes_to_alloc);