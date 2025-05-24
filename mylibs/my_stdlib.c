
#include "my_stdlib.h"



extern char *vidptr;
extern struct Cursor cursor;

void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		vidptr[cursor.loc++] = str[i++];
		vidptr[cursor.loc++] = 0x07;
	}
	refresh_cursor();
}

void kprint(const int)
{
    
}

void kprint_newline()
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	cursor.loc = cursor.loc + (line_size - cursor.loc % (line_size));
	refresh_cursor();
}

extern volatile char *char_buffer;
char kgetc()
{
    *char_buffer = 0; // clear buffer

    while (*char_buffer == 0){ 
        asm volatile("pause"); // waiting for a char from the keyboard 
    }

    return *char_buffer;
}

void kscanf (char* str, const unsigned int* buffer_size)
{
    char tmp_char;
    for (int i = 0; i < *buffer_size-1; i++)
    {
        tmp_char = kgetc();
        if(tmp_char == '\n') {
            str[i] = '\0';          //waiting for an "enter" key and adding null-terminator
            return;
        }

        str[i] = tmp_char;
    }
    str[*buffer_size-1] = '\0'; //null terminator at the end
    
}

char* strchr(const char* str, int ch) {
    while(*str != '\0') {
        if(*str == (char)ch) {
            return (char*)str;  // Знайшли символ - повертаємо вказівник
        }
        str++;
    }
    // Якщо символ не знайдено і це нуль-символ
    if((char)ch == '\0') {
        return (char*)str;
    }
    return NULL;  // Символ не знайдено
}