#include <stdint.h>
#include "uart.h"
#include "trap.h"
#include "scheduler.h"
#include "timer.h"
#include "fs.h"

void kmain(void) {
    uart_init();
    uart_puts("RISC-V Teaching Kernel starting (with preemption)...\n");

    extern void trap_vector(void);
    uintptr_t stvec = (uintptr_t)&trap_vector;
    asm volatile("csrw stvec, %0" :: "r"(stvec));

    scheduler_init();
    timer_init();
    fs_init();

    extern void shell_run(void);
    scheduler_spawn(shell_run);

    extern void user_prog_hello(void);
    extern void user_prog_echo(void);
    extern void user_prog_fstest(void);
    scheduler_spawn(user_prog_hello);
    scheduler_spawn(user_prog_echo);
    scheduler_spawn(user_prog_fstest);

    scheduler_run();

    uart_puts("No more ready tasks - kernel idle\n");
    while (1) asm volatile("wfi");
}
