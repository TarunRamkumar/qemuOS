#ifndef STRING_H
#define STRING_H

#include <stdint.h>

void *memset(void *dst, int c, uint64_t n);
void *memcpy(void *dst, const void *src, uint64_t n);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, uint64_t n);
uint64_t strlen(const char *s);

#endif
