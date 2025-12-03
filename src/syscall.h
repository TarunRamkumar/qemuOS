#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#define SYS_YIELD 1
#define SYS_WRITE 2
#define SYS_SPAWN 3

int do_sys_write(const char *s, int len);
void do_sys_yield(void);
int do_sys_spawn(void (*entry)(void));

#endif
