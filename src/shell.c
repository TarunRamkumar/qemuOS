#include "uart.h"
#include "fs.h"
#include "syscall.h"
#include <string.h>
#include <stdint.h>

#define LINE_MAX 80
#define FD_READ 0x1
#define FD_WRITE 0x2

extern void user_prog_hello(void);
extern void user_prog_echo(void);
extern void user_prog_fstest(void);

// Helper function to make system calls
static inline int syscall(int num, uint64_t a0, uint64_t a1, uint64_t a2) {
    register long a7 asm("a7") = num;
    register long a0_reg asm("a0") = a0;
    register long a1_reg asm("a1") = a1;
    register long a2_reg asm("a2") = a2;
    asm volatile("ecall" : "+r"(a0_reg) : "r"(a7), "r"(a1_reg), "r"(a2_reg) : "memory");
    return (int)a0_reg;
}

void shell_run(void) {
    char line[LINE_MAX];
    int pos = 0;

    uart_puts("\nSimple RISC-V Shell\n> ");

    while (1) {
        char c = uart_getc_block();

        if (c == '\r' || c == '\n') {
            line[pos] = 0;
            uart_puts("\n");

            if (strcmp(line, "ls") == 0) {
                char buf[256];
                fs_list_files(buf, sizeof(buf));
                uart_puts(buf);
            } else if (strncmp(line, "run ", 4) == 0) {
                const char *name = line + 4;
                if (strcmp(name, "hello") == 0) do_sys_spawn(user_prog_hello);
                else if (strcmp(name, "echo") == 0) do_sys_spawn(user_prog_echo);
                else if (strcmp(name, "fstest") == 0) do_sys_spawn(user_prog_fstest);
                else uart_puts("unknown program\n");
            } else if (strncmp(line, "cat ", 4) == 0) {
                const char *name = line + 4;
                int fd = syscall(4, (uint64_t)name, FD_READ, 0);  // SYS_OPEN
                if (fd < 0) {
                    uart_puts("Error: file not found\n");
                } else {
                    char buf[256];
                    int total_read = 0;
                    while (1) {
                        int n = syscall(5, fd, (uint64_t)(buf + total_read), 256 - total_read);  // SYS_READ
                        if (n <= 0) break;
                        total_read += n;
                        if (total_read >= 255) break;
                    }
                    buf[total_read] = 0;
                    uart_puts(buf);
                    if (total_read > 0 && buf[total_read - 1] != '\n') {
                        uart_puts("\n");
                    }
                    syscall(7, fd, 0, 0);  // SYS_CLOSE
                }
            } else if (strncmp(line, "create ", 7) == 0) {
                const char *name = line + 7;
                int result = syscall(8, (uint64_t)name, 0, 0);  // SYS_CREATE
                if (result == 0) {
                    uart_puts("File created\n");
                } else {
                    uart_puts("Error: failed to create file\n");
                }
            } else if (strncmp(line, "delete ", 7) == 0) {
                const char *name = line + 7;
                int result = syscall(9, (uint64_t)name, 0, 0);  // SYS_DELETE
                if (result == 0) {
                    uart_puts("File deleted\n");
                } else {
                    uart_puts("Error: file not found\n");
                }
            } else if (strncmp(line, "write ", 6) == 0) {
                // Format: write <filename> <text>
                const char *rest = line + 6;
                char filename[32];
                int i = 0;
                while (rest[i] && rest[i] != ' ' && i < 31) {
                    filename[i] = rest[i];
                    i++;
                }
                filename[i] = 0;
                
                if (rest[i] == ' ') {
                    const char *text = rest + i + 1;
                    int fd = syscall(4, (uint64_t)filename, FD_WRITE, 0);  // SYS_OPEN
                    if (fd < 0) {
                        uart_puts("Error: file not found (create it first)\n");
                    } else {
                        int len = strlen(text);
                        int written = syscall(6, fd, (uint64_t)text, len);  // SYS_WRITE_FD
                        if (written > 0) {
                            uart_puts("Written to file\n");
                        } else {
                            uart_puts("Error: write failed\n");
                        }
                        syscall(7, fd, 0, 0);  // SYS_CLOSE
                    }
                } else {
                    uart_puts("Usage: write <filename> <text>\n");
                }
            } else if (strcmp(line, "help") == 0) {
                uart_puts("Commands:\n");
                uart_puts("  ls              - List files\n");
                uart_puts("  cat <file>      - Display file contents\n");
                uart_puts("  create <file>   - Create new file\n");
                uart_puts("  delete <file>   - Delete file\n");
                uart_puts("  write <file> <text> - Write text to file\n");
                uart_puts("  run <prog>      - Run program\n");
                uart_puts("  help            - Show this help\n");
            }

            pos = 0;
            uart_puts("> ");
        } else {
            if (pos < LINE_MAX-1) {
                uart_putc(c);
                line[pos++] = c;
            }
        }
    }
}
