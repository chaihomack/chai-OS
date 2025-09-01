#pragma once

#include <stdint.h>

#define inb(port) ({               \
    uint8_t _v;                    \
    __asm__ __volatile__ ("inb %1, %0" \
                          : "=a" (_v)   \
                          : "Nd" (port)); \
    _v;                            \
})

#define outb(port, val)             \
    __asm__ __volatile__ ("outb %0, %1" \
                          :            \
                          : "a" (val), "Nd" (port))

#define inw(port) ({               \
    uint16_t _v;                   \
    __asm__ __volatile__ ("inw %1, %0" \
                          : "=a" (_v) \
                          : "Nd" (port)); \
    _v;                            \
})

#define outw(port, val)             \
    __asm__ __volatile__ ("outw %0, %1" \
                          :            \
                          : "a" (val), "Nd" (port))