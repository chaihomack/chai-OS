#pragma once

#include<stdint.h>

void* kmalloc(uint16_t bytes_to_alloc);
void* kcalloc(uint16_t bytes_to_alloc);