#pragma once

#include "page_structures.h"

void* create_pdt(page_record *first_pt);
void* create_pt(uint32_t mapping_start_addr);
void* page_alloc();
void clear_page(void *page);

void* map_full_memory_pdt();