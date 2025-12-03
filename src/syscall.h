#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#define SYS_YIELD 1
#define SYS_WRITE 2
#define SYS_SPAWN 3
#define SYS_OPEN 4
#define SYS_READ 5
#define SYS_WRITE_FD 6
#define SYS_CLOSE 7
#define SYS_CREATE 8
#define SYS_DELETE 9
#define SYS_SEEK 10

int do_sys_write(const char *s, int len);
void do_sys_yield(void);
int do_sys_spawn(void (*entry)(void));
int do_sys_open(const char *name, int flags);
int do_sys_read(int fd, char *buf, int len);
int do_sys_write_fd(int fd, const char *buf, int len);
int do_sys_close(int fd);
int do_sys_create(const char *name);
int do_sys_delete(const char *name);
int do_sys_seek(int fd, int offset);

#endif
