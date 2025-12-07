#include "string.h"

// Fills the first n bytes of the memory area pointed to by dst with the constant byte c.
void *memset(void *dst, int c, uint64_t n) {
    unsigned char *p = dst;
    while (n--) *p++ = (unsigned char)c;
    return dst;
}

// Copies n bytes from memory area src to memory area dst.
void *memcpy(void *dst, const void *src, uint64_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dst;
}

// Compares the two strings a and b.
int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) {
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

// Compares at most the first n bytes of a and b.
int strncmp(const char *a, const char *b, uint64_t n) {
    while (n-- && *a && (*a == *b)) {
        a++; b++;
    }
    return (n == (uint64_t)-1) ? 0 : (unsigned char)*a - (unsigned char)*b;
}

// Calculates the length of the string s, excluding the terminating null byte.
uint64_t strlen(const char *s) {
    uint64_t n = 0;
    while (s[n]) n++;
    return n;
}

// Copies up to n characters from the string pointed to, by src to dest.
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
