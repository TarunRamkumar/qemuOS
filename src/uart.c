#include "uart.h"
#include <stdint.h>

// SBI call numbers for console functions.
#define SBI_CONSOLE_PUTCHAR 1
#define SBI_CONSOLE_GETCHAR 2

// Makes a Supervisor Binary Interface (SBI) call.
// 'which' is the SBI call number, 'arg0' is the first argument.
static inline long sbi_call(long which, long arg0) {
    register long a0 asm("a0") = arg0;
    register long a7 asm("a7") = which;
    // ecall transfers control to the supervisor (M-mode).
    asm volatile ("ecall" : "=r"(a0) : "0"(a0), "r"(a7) : "memory");
    return a0;
}

// Initializes the UART. In this SBI-based implementation, it's a no-op
// as the supervisor is expected to handle hardware initialization.
void uart_init(void) {
    /* nothing */
}

// Outputs a single character to the console via SBI call.
void uart_putc(char c) {
    sbi_call(SBI_CONSOLE_PUTCHAR, c);
}

// Outputs a null-terminated string to the console.
// Translates '\n' to '\r\n' for proper terminal display.
void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') uart_putc('\r');
        uart_putc(*s++);
    }
}

// Blocks until a character is received from the console via SBI call.
char uart_getc_block(void) {
    long c = -1;
    // SBI_CONSOLE_GETCHAR returns -1 if no character is available.
    while (c == -1) {
        c = sbi_call(SBI_CONSOLE_GETCHAR, 0);
    }
    return (char)c;
}
