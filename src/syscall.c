#include "syscall.h"
#include "uart.h"
#include "scheduler.h"

int do_sys_write(const char *s, int len) {
    for (int i = 0; i < len; i++)
        uart_putc(s[i]);
    return len;
}

void do_sys_yield(void) {
    scheduler_yield();
}

int do_sys_spawn(void (*entry)(void)) {
    return scheduler_spawn(entry);
}
