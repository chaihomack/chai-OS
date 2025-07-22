#include "../drivers/disk_driver/disk_driver.h"
#include <stdint.h>
#include "../drivers/keyboard/keyboard_driver.h"
#include "../mylibs/my_stdlib.h"
#include "../memory_management/heap.h"
#include "../memory_management/paging.h"

#define KERNEL_ADDRESS_START 0x100000

uint32_t get_free_space_after_kernel_index()
{
    BYTE buffer[512];
    for (size_t i = 0; i < 99999999; i++)
        {
            ATA_disk_read(buffer, i, 1);
            if(*(uint32_t*)(buffer + 508) == 0xDEADBEEF){
                return i + 1;
            }
        }
        return 0; // no signature
}

uint32_t* get_address_after_kernel(){
    for (uint32_t* i = (uint32_t*)KERNEL_ADDRESS_START; (uint32_t)i < 99999999; i += 1024)
    {
        if (*(i + 1023) == 0xDEADBEEF) {
            return (i + 1024);
        }
    }
    return NULL; // error, cant find end
}

extern void mmu_setup(void* pdt_ptr);

void kmain() 
{
    idt_init();
    kb_init();
    
    clear_screen();

    void *pdt_ptr = map_full_memory_pdt();

    mmu_setup(pdt_ptr);
    kprint_str("MMU setup done");
    kprint_int((uint32_t)get_address_after_kernel());
    
    kprint_newline();
    int *yo = kmalloc(32);
    int *nya = kmalloc(32);
    kprint_int((int32_t)yo);
    kprint_newline();
    kprint_int((int32_t)nya);
    kprint_newline();
    *yo = 123;
    *nya = 2;
    kprint_int(*yo + *nya);
}


