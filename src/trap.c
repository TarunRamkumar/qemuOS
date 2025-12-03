#include "trap.h"
#include "uart.h"
#include "syscall.h"
#include "timer.h"
#include "scheduler.h"
#include <stdint.h>

#define TF_A0 48
#define TF_A1 56
#define TF_A7 104
#define TF_SEPC 224

static inline uint64_t read_scause(void) {
    uint64_t x; asm volatile("csrr %0, scause":"=r"(x)); return x;
}

void handle_trap_from_asm(uint64_t *tf) {
    uint64_t scause = read_scause();
    uint64_t sepc = tf[TF_SEPC/8];

    int is_interrupt = (scause >> 63) & 1;
    uint64_t code = scause & 0xff;

    if (is_interrupt) {
        if (code == 5) {  
            timer_handle_irq();
            return;
        }
    } else {
        if (code == 8 || code == 9) {
            uint64_t num = tf[TF_A7/8];

            if (num == SYS_YIELD) {
                tf[TF_SEPC/8] = sepc + 4;
                scheduler_yield_from_trap();
                return;
            } else if (num == SYS_WRITE) {
                const char *buf = (const char *)tf[TF_A0/8];
                int len = tf[TF_A1/8];
                do_sys_write(buf, len);
                tf[TF_SEPC/8] = sepc + 4;
                return;
            } else if (num == SYS_SPAWN) {
                void (*entry)(void) = (void (*)(void))tf[TF_A0/8];
                int pid = do_sys_spawn(entry);
                tf[TF_A0/8] = pid;
                tf[TF_SEPC/8] = sepc + 4;
                return;
            }
        }
    }

    uart_puts("Unhandled trap!\n");
    while (1) asm volatile("wfi");
}
