#pragma once

#include <stdint.h>
#include "../drivers/keyboard/key_codes.h"

#define LINES 25
#define COLUMNS_IN_LINE 80

void kprint_char(const char character);
void kprint_char_no_ref_cursor(const char character);
void kprint_str(const char *str);
void kprint_int(const int32_t num);
void kback_space();

void kprint_newline();
char kgetc();
char kgetc_plus_code(uint16_t *ret_scan_code);
void kscanf (char* str, const uint32_t buffer_size);

void clear_screen();
void refresh_cursor();
void set_cursor_pos(uint16_t line, uint16_t column);
void set_cursor_pos_not_visible(uint16_t line, uint16_t column);

char kgetc_with_hotkey_support(const uint8_t scan_code_for_hotkey, uint8_t *is_pressed, uint8_t* ret_scan_code);