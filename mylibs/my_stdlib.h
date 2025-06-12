#pragma once

#include <stdint.h>
#include "../drivers/keyboard/keyboard_driver.h"

typedef unsigned char   BYTE;
typedef unsigned char   uchar;
typedef unsigned short  WORD;
typedef uint32_t    DWORD;
typedef uint32_t    size_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

void kprint_char(const unsigned char character);
void kprint_str(const char *str);
void kprint_int(const int num);
int ipow(int base, int exponent); //will be in mathlib later

int kstrcmp(const char* s1, const char* s2);
char* kstrchr(const char* str, int ch);

void kprint_newline();
char kgetc();
void kscanf (char* str, const unsigned int* buffer_size);

int is_bit_set(BYTE byte, int bit);
BYTE toggle_bit(BYTE byte, int bit);

int strlen(const char *str);
void *memcpy(void *dest, const void *src, unsigned int n);
void *memset(void *s, int c, unsigned int n);
char *strchr(const char *s, int c);