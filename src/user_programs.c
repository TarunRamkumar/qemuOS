#include "syscall.h"
#include "fs.h"
#include <stdint.h>

const char _prog_hello[] = "Hello from embedded program!\n";
const char _prog_echo[]  = "Echo program running.\n";

// Helper function to make system calls
static inline int syscall(int num, uint64_t a0, uint64_t a1, uint64_t a2) {
    register long a7 asm("a7") = num;
    register long a0_reg asm("a0") = a0;
    register long a1_reg asm("a1") = a1;
    register long a2_reg asm("a2") = a2;
    asm volatile("ecall" : "+r"(a0_reg) : "r"(a7), "r"(a1_reg), "r"(a2_reg) : "memory");
    return (int)a0_reg;
}

void user_prog_hello(void) {
    int len;
    const char *s = fs_get_file_content("hello", &len);
    if (s) do_sys_write(s, len);
    do_sys_yield();
}

void user_prog_echo(void) {
    int len;
    const char *s = fs_get_file_content("echo", &len);
    if (s) do_sys_write(s, len);
    do_sys_yield();
}

// New program demonstrating file descriptor API
void user_prog_fstest(void) {
    do_sys_write("File system test program\n", 25);
    
    // Try to open and read a file using file descriptors
    int fd = syscall(4, (uint64_t)"hello", 0x1, 0);  // SYS_OPEN with FD_READ
    if (fd >= 0) {
        char buf[128];
        int n = syscall(5, fd, (uint64_t)buf, 127);  // SYS_READ
        if (n > 0) {
            buf[n] = 0;
            do_sys_write("Read from file: ", 16);
            do_sys_write(buf, n);
        }
        syscall(7, fd, 0, 0);  // SYS_CLOSE
    }
    
    do_sys_yield();
}
