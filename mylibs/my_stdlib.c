
#include "my_stdlib.h"

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
