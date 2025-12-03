#include "uart.h"
#include <stdint.h>

#define SBI_CONSOLE_PUTCHAR 1
#define SBI_CONSOLE_GETCHAR 2

static inline long sbi_call(long which, long arg0) {
    register long a0 asm("a0") = arg0;
    register long a7 asm("a7") = which;
    asm volatile ("ecall" : "=r"(a0) : "0"(a0), "r"(a7) : "memory");
    return a0;
}

void uart_init(void) {
    /* nothing */
}

void uart_putc(char c) {
    sbi_call(SBI_CONSOLE_PUTCHAR, c);
}

void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') uart_putc('\r');
        uart_putc(*s++);
    }
}

char uart_getc_block(void) {
    long c = -1;
    while (c == -1) {
        c = sbi_call(SBI_CONSOLE_GETCHAR, 0);
    }
    return (char)c;
}
