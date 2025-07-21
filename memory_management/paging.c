#include "paging.h"

#include "heap.h"
#include "../mylibs/my_stdtypes.h"
#include "stdint.h"
#include "page_structures.h"

extern void mmu_setup(uint32_t* pdt_ptr);

#define PAGE_SIZE  4096 
#define FRAME_SIZE PAGE_SIZE
#define ENTRIES_PER_TABLE 1024

void* create_pdt(page_record *first_pt)
{
        void *pdt = kmalloc(PAGE_SIZE);
        clear_page(pdt);
        page_record *pde = (page_record*)pdt;
        pde->present = 1;
        pde->rw = 1;
        pde->user_access = 0;
        pde->phys_addr = ((uint32_t)first_pt) >> 12; 
        return pdt;
}

void* create_pt(uint32_t mapping_start_addr)
{
        void *pt = kmalloc(PAGE_SIZE);
        clear_page(pt);
        uint32_t phys_addr = (mapping_start_addr + 4095) >> 12;

        page_record *pte = (page_record*)pt;
        for (uint32_t i = 0; i < ENTRIES_PER_TABLE; i++) {
                pte[i].present = 1;
                pte[i].rw = 1;
                pte[i].user_access = 0;
                pte[i].phys_addr = phys_addr++;
        }
        return pt;
}

void* page_alloc()
{
        void *page = kmalloc(PAGE_SIZE);
        clear_page(page);
        return page;
}

void clear_page(void *page)
{
        // 4096/4 = 1024
        for (size_t i = 0; i < PAGE_SIZE/4; i++)
        {
                ((int32_t*)page)[i] = 0;
        }
}