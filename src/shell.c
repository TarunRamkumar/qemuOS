#include "uart.h"
#include "fs.h"
#include "syscall.h"
#include <string.h>

#define LINE_MAX 80

extern void user_prog_hello(void);
extern void user_prog_echo(void);

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
                char buf[128];
                fs_list_files(buf, sizeof(buf));
                uart_puts(buf);
            } else if (strncmp(line, "run ", 4) == 0) {
                const char *name = line + 4;
                if (strcmp(name, "hello") == 0) do_sys_spawn(user_prog_hello);
                else if (strcmp(name, "echo") == 0) do_sys_spawn(user_prog_echo);
                else uart_puts("unknown program\n");
            } else if (strcmp(line, "help") == 0) {
                uart_puts("Commands: ls, run <file>, help\n");
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
