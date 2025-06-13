
#include "my_stdlib.h"

#include "../drivers/keyboard/keyboard_driver.h"

extern char *vidptr;
extern struct Cursor cursor;

void kprint_char(const uchar character)
{
    vidptr[cursor.loc++] = character;
    vidptr[cursor.loc++] = 0x07;
}

void kprint_str(const uchar *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
        kprint_char(str[i++]);
	}
	refresh_cursor();
}

int ipow(int base, int exponent) {
    int result = 1;

    if (exponent < 0) {
        return 0;
    }

    while (exponent > 0) {
        if (exponent % 2 == 1) {
            result *= base;
        }
        base *= base;
        exponent /= 2;
    }

    return result;
}

void kprint_int(const int32_t num)
{
    int cpy_of_num = num, size_of_num = 0;

    if(num < 0)
    {
        kprint_char('-');
        cpy_of_num *= -1;        
    }

    uchar nums[10] = {"0123456789"};

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

void kscanf (uchar* str, const uint32_t* buffer_size)
{
    uchar tmp_char;
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

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {       s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int is_bit_set(BYTE byte, int bit) {
    return (byte & (1 << bit)) ? 1 : 0;
}

BYTE toggle_bit(BYTE byte, int bit) {
    return byte ^ (1 << bit);
}

int strlen(const char *str) {
    int len = 0;
    while (*str++) len++;
    return len;
}

void *memcpy(void *dest, const void *src, unsigned int n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

void *memset(void *s, int c, unsigned int n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) {
            return (char *)s;
        }
        s++;
    }
    
    if (c == 0) {
        return (char *)s;
    }
    return NULL;
}
