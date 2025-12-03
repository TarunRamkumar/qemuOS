#include "syscall.h"
#include "uart.h"
#include "scheduler.h"
#include "fs.h"

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

int do_sys_open(const char *name, int flags) {
    return fs_open(name, flags);
}

int do_sys_read(int fd, char *buf, int len) {
    return fs_read(fd, buf, len);
}

int do_sys_write_fd(int fd, const char *buf, int len) {
    return fs_write(fd, buf, len);
}

int do_sys_close(int fd) {
    return fs_close(fd);
}

int do_sys_create(const char *name) {
    return fs_create(name);
}

int do_sys_delete(const char *name) {
    return fs_delete(name);
}

int do_sys_seek(int fd, int offset) {
    return fs_seek(fd, offset);
}
