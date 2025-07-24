#pragma once

#include <stdint.h>

void kprint_char(const char character);
void kprint_str(const char *str);
void kprint_int(const int32_t num);

void kprint_newline();
char kgetc();
void kscanf (char* str, const uint32_t buffer_size);

void clear_screen();
void refresh_cursor();
void set_cursor_pos(uint16_t pos);