#include "string.h"

void *memset(void *dst, int c, uint64_t n) {
    unsigned char *p = dst;
    while (n--) *p++ = (unsigned char)c;
    return dst;
}

void *memcpy(void *dst, const void *src, uint64_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dst;
}

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) {
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, uint64_t n) {
    while (n-- && *a && (*a == *b)) {
        a++; b++;
    }
    return (n == (uint64_t)-1) ? 0 : (unsigned char)*a - (unsigned char)*b;
}

uint64_t strlen(const char *s) {
    uint64_t n = 0;
    while (s[n]) n++;
    return n;
}

char *strncpy(char *dest, const char *src, uint64_t n) {
    uint64_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for ( ; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}
