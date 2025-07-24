#pragma once

#include"../memory_management/heap.h"
#include <stdint.h>
#include "my_stdtypes.h"
#include "../drivers/keyboard/keyboard_driver.h"

int ipow(int base, int exponent);

int is_bit_set(BYTE byte, int bit);
BYTE toggle_bit(BYTE byte, int bit);

void *memcpy(void *dest, const void *src, unsigned int n);
void *memset(void *s, int c, unsigned int n);

int strcmp(const char* s1, const char* s2);
int strlen(const char *str);
char *strchr(const char *s, int c);