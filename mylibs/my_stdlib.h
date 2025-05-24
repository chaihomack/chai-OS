#pragma once

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    DWORD;
#ifndef NULL
#define NULL ((void*)0)
#endif

#include "../drivers/keyboard/keyboard_driver.h"

void kprint(const char *str);
void kprint_newline();
char kgetc();
void kscanf (char* str, const unsigned int* buffer_size);
