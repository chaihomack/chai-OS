#include "../mylibs/my_stdtypes.h"
#include "disk_driver_read_only.c"

void kernel_load()
{
    ATA_disk_init();

    BYTE *kernel_code_ptr = (BYTE*)0x100000;

    size_t kernel_start_index;

    BYTE buff[512];
    for (size_t i = 0; i < 999999999; i++){
        ATA_disk_read(buff, i, 1);
        if(*(uint32_t*)buff == 0x1BADB002){
            kernel_start_index = i; // + 1 because its end of boot sec stage. so + 1 its kernel
            break;
        }    
    }
    
    for (size_t i = kernel_start_index; i < 99999999; i++)
    {
        ATA_disk_read(kernel_code_ptr, i, 1);
        if(*(uint32_t*)(kernel_code_ptr + 508) == 0xDEADBEEF){
            break;
        }
        kernel_code_ptr += 512;
    }
    asm volatile ("jmp 0x100100");
}