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
    for (BYTE* i = (BYTE*)KERNEL_ADDRESS_START; (uint32_t)i < 99999999; i += 4096)
    {
        if (*(uint32_t*)(i + 4092) == 0xDEADBEEF) {
            return (uint32_t*)(i += 4096);
        }
    }
    return 0; // error, cant find end
}

void* kmalloc(uint16_t bytes_to_alloc);

extern void mmu_setup(uint32_t* pdt_ptr);

void kmain() 
{
    idt_init();
    kb_init();
    
    clear_screen();

    void *pdt_ptr = create_pdt(create_pt(0));
    int *yo = kmalloc(4096);
    int *nya = kmalloc(4096);
    kprint_int((int32_t)yo);
    kprint_newline();
    kprint_int((int32_t)nya);
    
    *yo = 123;
    *nya = 2;
    kprint_int(*yo + *nya);
}


