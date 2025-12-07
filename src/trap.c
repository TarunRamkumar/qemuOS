#include "trap.h"
#include "uart.h"
#include "syscall.h"
#include "timer.h"
#include "scheduler.h"
#include <stdint.h>

// Defines for accessing registers in the trap frame.
// These are byte offsets from the stack pointer.
#define TF_A0 48
#define TF_A1 56
#define TF_A2 64
#define TF_A7 104
#define TF_SEPC 224

// Reads the scause (Supervisor Cause) register.
static inline uint64_t read_scause(void) {
    uint64_t x; asm volatile("csrr %0, scause":"=r"(x)); return x;
}

// C-level trap handler called from trap_entry.S
void handle_trap_from_asm(uint64_t *tf) {
    // Read the cause of the trap and the instruction that caused it.
    uint64_t scause = read_scause();
    uint64_t sepc = tf[TF_SEPC/8];

    // Determine if it's an interrupt or an exception.
    int is_interrupt = (scause >> 63) & 1;
    uint64_t code = scause & 0xff;

    if (is_interrupt) {
        // Handle interrupts.
        if (code == 5) {  // Supervisor Timer Interrupt
            timer_handle_irq();
            return;
        }
    } else {
        // Handle exceptions (e.g., syscalls).
        if (code == 8 || code == 9) { // Environment call from U-mode or S-mode (syscall)
            // Get syscall number from a7.
            uint64_t num = tf[TF_A7/8];

            if (num == SYS_YIELD) {
                tf[TF_SEPC/8] = sepc + 4; // Advance past ecall instruction
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
                tf[TF_A0/8] = pid; // Return PID in a0
                tf[TF_SEPC/8] = sepc + 4;
                return;
            } else if (num == SYS_OPEN) {
                const char *name = (const char *)tf[TF_A0/8];
                int flags = tf[TF_A1/8];
                int fd = do_sys_open(name, flags);
                tf[TF_A0/8] = fd; // Return file descriptor in a0
                tf[TF_SEPC/8] = sepc + 4;
                return;
            } else if (num == SYS_READ) {
                int fd = tf[TF_A0/8];
                char *buf = (char *)tf[TF_A1/8];
                int len = tf[TF_A2/8];
                int result = do_sys_read(fd, buf, len);
                tf[TF_A0/8] = result; // Return result in a0
                tf[TF_SEPC/8] = sepc + 4;
                return;
            } else if (num == SYS_WRITE_FD) {
                int fd = tf[TF_A0/8];
                const char *buf = (const char *)tf[TF_A1/8];
                int len = tf[TF_A2/8];
                int result = do_sys_write_fd(fd, buf, len);
                tf[TF_A0/8] = result; // Return result in a0
                tf[TF_SEPC/8] = sepc + 4;
                return;
            } else if (num == SYS_CLOSE) {
                int fd = tf[TF_A0/8];
                int result = do_sys_close(fd);
                tf[TF_A0/8] = result; // Return result in a0
                tf[TF_SEPC/8] = sepc + 4;
                return;
            } else if (num == SYS_CREATE) {
                const char *name = (const char *)tf[TF_A0/8];
                int result = do_sys_create(name);
                tf[TF_A0/8] = result; // Return result in a0
                tf[TF_SEPC/8] = sepc + 4;
                return;
            } else if (num == SYS_DELETE) {
                const char *name = (const char *)tf[TF_A0/8];
                int result = do_sys_delete(name);
                tf[TF_A0/8] = result; // Return result in a0
                tf[TF_SEPC/8] = sepc + 4;
                return;
            } else if (num == SYS_SEEK) {
                int fd = tf[TF_A0/8];
                int offset = tf[TF_A1/8];
                int result = do_sys_seek(fd, offset);
                tf[TF_A0/8] = result; // Return result in a0
                tf[TF_SEPC/8] = sepc + 4;
                return;
            }
        }
    }

    // If we get here, it's an unhandled trap.
    uart_puts("Unhandled trap!\n");
    // Halt the system.
    while (1) asm volatile("wfi");
}
