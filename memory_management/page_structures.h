#pragma once


#include<stdint.h>


typedef struct __attribute__((packed)) {
    uint32_t present     : 1;
    uint32_t rw          : 1;
    uint32_t user_access : 1;
    uint32_t pwt         : 1;
    uint32_t pcd         : 1;
    uint32_t accessed    : 1;
    uint32_t dirty       : 1;
    uint32_t pat         : 1;
    uint32_t global      : 1;
    uint32_t available   : 3;
    uint32_t phys_addr   : 20;  //this address * 4096 = real phys address
} page_record;

// 1 page stores 512 record abt frames so 1 page can store info abt 2097152B or 2048KiB or 2MiB of frames