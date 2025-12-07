#include "syscall.h"
#include "uart.h"
#include "scheduler.h"
#include "fs.h"

// System call to write a string to the console.
int do_sys_write(const char *s, int len) {
    for (int i = 0; i < len; i++)
        uart_putc(s[i]);
    return len;
}

// System call to yield the CPU to another process.
void do_sys_yield(void) {
    scheduler_yield();
}

// System call to spawn a new process.
int do_sys_spawn(void (*entry)(void)) {
    return scheduler_spawn(entry);
}

// System call to open a file.
int do_sys_open(const char *name, int flags) {
    return fs_open(name, flags);
}

// System call to read from a file descriptor.
int do_sys_read(int fd, char *buf, int len) {
    return fs_read(fd, buf, len);
}

// System call to write to a file descriptor.
int do_sys_write_fd(int fd, const char *buf, int len) {
    return fs_write(fd, buf, len);
}

// System call to close a file descriptor.
int do_sys_close(int fd) {
    return fs_close(fd);
}

// System call to create a file.
int do_sys_create(const char *name) {
    return fs_create(name);
}

// System call to delete a file.
int do_sys_delete(const char *name) {
    return fs_delete(name);
}

// System call to change the file offset.
int do_sys_seek(int fd, int offset) {
    return fs_seek(fd, offset);
}
