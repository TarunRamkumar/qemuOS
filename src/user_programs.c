#include "syscall.h"
#include "fs.h"

const char _prog_hello[] = "Hello from embedded program!\n";
const char _prog_echo[]  = "Echo program running.\n";

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
