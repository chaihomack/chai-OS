#pragma once

#include <stdint.h>
typedef unsigned char   BYTE;
typedef unsigned char   uchar;
typedef unsigned short  WORD;
typedef unsigned int    DWORD;
#ifndef NULL
#define NULL ((void*)0)
#endif

void kprint_char(const unsigned char character);
void kprint_str(const char *str);
void kprint_int(const int num);
int ipow(int base, int exponent); //will be in mathlib later

void kprint_newline();
char kgetc();
void kscanf (char* str, const unsigned int* buffer_size);
