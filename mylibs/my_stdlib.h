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

void kprint_char(const uchar character);
void kprint_str(const uchar *str);
void kprint_int(const int32_t num);
int ipow(int base, int exponent); //will be in mathlib later
char* kstrchr(const char* str, int ch);

void kprint_newline();
char kgetc();
void kscanf (uchar* str, const uint32_t* buffer_size);

int is_bit_set(BYTE byte, int bit);
BYTE toggle_bit(BYTE byte, int bit);

void *memcpy(void *dest, const void *src, unsigned int n);
void *memset(void *s, int c, unsigned int n);

int strcmp(const char* s1, const char* s2);
int strlen(const char *str);
char *strchr(const char *s, int c);