#include "kernelio.h"
#include <stdint.h>
#include "../drivers/keyboard/keyboard_driver.h"
#include "../mylibs/my_stdlib.h"
#include "../drivers/keyboard/key_codes.h"

/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

volatile char *vidptr = (char*)0xb8000;

struct Cursor	
{
        unsigned int loc;
};

struct Cursor cursor =
{
        .loc = 0
};

void refresh_cursor() {
        write_port(0x3D4, 0x0F); 
        write_port(0x3D5, (unsigned int)((cursor.loc/2) & 0xFF));
        
        write_port(0x3D4, 0x0E); 
        write_port(0x3D5, (unsigned int)(((cursor.loc/2) >> 8) & 0xFF));
}

void set_cursor_pos(uint16_t line, uint16_t column) {
        write_port(0x3D4, 0x0F);
        write_port(0x3D5, (uint8_t)((line*COLUMNS_IN_LINE +column) & 0xFF));

        write_port(0x3D4, 0x0E);
        write_port(0x3D5, (uint8_t)(((line*COLUMNS_IN_LINE +column) >> 8) & 0xFF));
        cursor.loc = (line*COLUMNS_IN_LINE +column) * 2;
}

void set_cursor_pos_not_visible(uint16_t line, uint16_t column)
{
        cursor.loc = (line*COLUMNS_IN_LINE +column) * 2;
}

void clear_screen()
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = 0x07;
	}       
}

void kback_space()
{
        cursor.loc -= 2; // move back two bytes (character and attribute)
        vidptr[cursor.loc] = ' '; // clear the character
        vidptr[cursor.loc + 1] = 0x07; // reset the attribute
        refresh_cursor();
}

void kprint_char(const char character)
{
        vidptr[cursor.loc++] = character;
        vidptr[cursor.loc++] = 0x07;
        refresh_cursor();
}

void kprint_char_no_ref_cursor(const char character)
{
        vidptr[cursor.loc++] = character;
        vidptr[cursor.loc++] = 0x07;
        // refresh_cursor();
}

void kprint_str(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
                kprint_char(str[i++]);
	}
	refresh_cursor();
}

void kprint_newline()
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	cursor.loc = cursor.loc + (line_size - cursor.loc % (line_size));
	refresh_cursor();
}

void kprint_int(const int32_t num)
{
        int cpy_of_num = num, size_of_num = 0;

        if(num < 0)
        {
                kprint_char('-');
                cpy_of_num *= -1;        
        }

        char nums[10] = {"0123456789"};

        if(num == 0) {kprint_char('0'); return; }        //костылирование ;)

        int tmp = cpy_of_num;
        while(tmp != 0)
        {
                tmp /= 10;
                size_of_num++;
        }
        
        
        for (int i = size_of_num; i > 0; --i)
        {
                int number = (cpy_of_num / ipow(10, i - 1))%10;
                kprint_char(nums[number]);
        }
    
}

extern volatile char char_buffer;
char kgetc()
{
        char_buffer = 0;

        while (char_buffer == 0){ 
                asm volatile("pause"); // waiting for a char from the keyboard 
        }

        return char_buffer;
}

extern volatile uint16_t scan_code;
char kgetc_plus_code(uint16_t *ret_scan_code)
{
        char_buffer = 0;
        scan_code = 0;

        while (char_buffer == 0 && scan_code == 0){ 
                asm volatile("pause"); // waiting for a char from the keyboard 
        }
        *ret_scan_code = scan_code;
        return char_buffer;
}

// if key is pressed, scan_code_for_hotkey will be 1 and if not - 0
char kgetc_with_hotkey_support(const uint8_t scan_code_for_hotkey, uint8_t *is_pressed, uint8_t* ret_scan_code)
{
        char_buffer = 0; // clear buffer
        scan_code = 0;

        while (char_buffer == 0 && scan_code == 0){ 
                asm volatile("pause"); // waiting for a char from the keyboard 
        }
        
        *is_pressed = 0;
        if(is_key_pressed(scan_code_for_hotkey))
                *is_pressed = 1;
        
        *ret_scan_code = scan_code;
        return char_buffer;
}

void kscanf(char* str, const uint32_t buffer_size)
{
        char tmp_char;
        int i = 0;

        while (i < buffer_size - 1) {
                tmp_char = kgetc();

                if (tmp_char == '\n') break;

                if (tmp_char == '\b') {
                if (i > 0) {
                        i--;
                        kback_space();
                }
                continue;
                }

                kprint_char(tmp_char);
                str[i++] = tmp_char;
        }

        str[i] = '\0';
}