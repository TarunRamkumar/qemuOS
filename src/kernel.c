#include <stdint.h>
#include "uart.h"
#include "trap.h"
#include "scheduler.h"
#include "timer.h"
#include "fs.h"

/*
 * The main function of the kernel.
 * This function is called by the assembly startup code in start.s
 */
void kmain(void) {
    /*
     * Initialize the UART (Universal Asynchronous Receiver/Transmitter)
     * for serial communication. This allows the kernel to print messages
     * to the console.
     */
    uart_init();
    uart_puts("RISC-V Teaching Kernel starting (with preemption).\n");

    /*
     * Set up the trap vector.
     * 'trap_vector' is a function defined in assembly (trap_entry.S) that
     * handles all traps (exceptions, interrupts, syscalls).
     * We get its address and write it to the 'stvec' (Supervisor Trap Vector)
     * register. From this point on, any trap will cause the CPU to jump to
     * the 'trap_vector' function.
     */
    extern void trap_vector(void);
    uintptr_t stvec = (uintptr_t)&trap_vector;
    asm volatile("csrw stvec, %0" :: "r"(stvec));

    /* Initialize the scheduler, which is responsible for managing tasks. */
    scheduler_init();

    /*
     * Initialize the timer. This is used for preemptive multitasking.
     * The timer will periodically generate interrupts, which will trigger
     * the scheduler to switch tasks.
     */
    timer_init();
    
    /* Initialize the file system. */
    fs_init();

    /* Spawn the initial tasks. */
    /* 'scheduler_spawn' adds a function to the scheduler's list of tasks to be run. */
    extern void shell_run(void);
    scheduler_spawn(shell_run); /* The interactive shell */

    /* Spawn some other user programs */
    extern void user_prog_hello(void);
    extern void user_prog_echo(void);
    extern void user_prog_fstest(void);
    scheduler_spawn(user_prog_hello);
    scheduler_spawn(user_prog_echo);
    scheduler_spawn(user_prog_fstest);

    /*
     * Start the scheduler. This function will start running the spawned tasks
     * and will not return unless there are no more tasks to run.
     */
    scheduler_run();

    /*
     * This part is reached only if scheduler_run() returns, which means
     * there are no more tasks in the ready state.
     */
    uart_puts("No more ready tasks - kernel idle\n");
    /*
     * Enter an infinite loop and wait for interrupts.
     * 'wfi' (Wait For Interrupt) is a low-power instruction that halts the CPU
     * until an interrupt occurs.
     */
    while (1) asm volatile("wfi");
}
