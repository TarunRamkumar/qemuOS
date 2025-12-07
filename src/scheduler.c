#include "scheduler.h"
#include "uart.h"
#include "string.h"

/*
 * This is a forward declaration for the context_switch function, which is
 * defined in assembly code (context_switch.S).
 * Note: This function is currently not used in this file. The task switching
 * is done via inline assembly with 'jr' instruction.
 */
extern void context_switch(uint64_t*, uint64_t*);

/* Static array to hold all the task control blocks (TCBs). */
static task_t tasks[MAX_TASKS];
/* Index of the currently running task in the 'tasks' array. -1 if no task is running. */
static int current = -1;

/* Initializes the scheduler. */
void scheduler_init(void) {
    /* Zero out the tasks array. */
    memset(tasks, 0, sizeof(tasks));
    /* No task is currently running. */
    current = -1;
}

/* Helper function to zero out the register context of a task. */
static void zero_regs(uint64_t *r) {
    /* 
     * The number 27 here corresponds to the number of registers to be saved.
     * This should match the context structure.
     */
    for (int i = 0; i < 27; i++) r[i] = 0;
}

/*
 * Spawns a new task.
 * Finds an empty slot in the tasks array and initializes a new task.
 * entry: A function pointer to the entry point of the task.
 * Returns the task ID (index in the tasks array) or -1 if no slot is available.
 */
int scheduler_spawn(void (*entry)(void)) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_EMPTY || tasks[i].state == TASK_EXITED) {
            tasks[i].state = TASK_READY;
            tasks[i].entry = entry;
            /* Set the stack pointer to the top of the allocated stack for this task. */
            tasks[i].sp = (uint64_t)&tasks[i].stack[1024];
            zero_regs(tasks[i].regs);
            /* Store the entry point in regs[0]. This is used by 'jr' to start the task. */
            tasks[i].regs[0] = (uint64_t)entry;
            return i;
        }
    }
    return -1; /* No available task slot. */
}

/*
 * Finds the next task in the READY state using a round-robin algorithm.
 * s: The index of the current task.
 * Returns the index of the next ready task, or -1 if no ready tasks are found.
 */
static int next_ready(int s) {
    for (int i = 1; i <= MAX_TASKS; i++) {
        int j = (s + i) % MAX_TASKS;
        if (tasks[j].state == TASK_READY) return j;
    }
    return -1;
}

/*
 * Yields the CPU to another task. This is for cooperative multitasking.
 * This implementation is very basic and simply jumps to the entry point of the next task.
 * It does not save the full context of the current task.
 */
void scheduler_yield(void) {
    int prev = current;
    int nxt = next_ready(current);
    if (nxt == -1) return; /* No other ready tasks. */

    if (prev == -1) {
        /* This case is for starting the very first task. */
        current = nxt;
        tasks[nxt].state = TASK_RUNNING;
        /* Set the stack pointer and jump to the new task's entry point. */
        asm volatile("mv sp, %0" :: "r"(tasks[nxt].sp));
        asm volatile("jr %0" :: "r"(tasks[nxt].regs[0]));
        return;
    }

    /* Save the current stack pointer. Note: this is not used to restore the stack later. */
    uint64_t sp;
    asm volatile("mv %0, sp" : "=r"(sp));
    tasks[prev].sp = sp;

    current = nxt;
    tasks[nxt].state = TASK_RUNNING;
    /* Set the stack pointer and jump to the new task's entry point. */
    asm volatile("mv sp, %0" :: "r"(tasks[nxt].sp));
    asm volatile("jr %0" :: "r"(tasks[nxt].regs[0]));
}

/*
 * Preempts the current task. Intended to be called from a trap (e.g., timer interrupt).
 * This implementation is very similar to scheduler_yield and has the same limitations.
 */
void scheduler_preempt(void) {
    int prev = current;
    int nxt = next_ready(current);
    if (nxt == -1) return; /* No other ready tasks. */
    if (prev == nxt) return; /* No need to switch to the same task. */

    uint64_t sp;
    asm volatile("mv %0, sp" : "=r"(sp));
    if (prev >= 0) tasks[prev].sp = sp;

    current = nxt;
    tasks[nxt].state = TASK_RUNNING;
    /* Set the stack pointer and jump to the new task's entry point. */
    asm volatile("mv sp, %0" :: "r"(tasks[nxt].sp));
    asm volatile("jr %0" :: "r"(tasks[nxt].regs[0]));
}

/* Alias for scheduler_preempt, to be called from a trap handler. */
void scheduler_yield_from_trap(void) {
    scheduler_preempt();
}

/*
 * Starts the scheduler.
 * This function loops through the tasks and runs them.
 * This implementation runs each task to completion.
 */
void scheduler_run(void) {
    while (1) {
        int any = 0;
        for (int i = 0; i < MAX_TASKS; i++) {
            if (tasks[i].state == TASK_READY) {
                any = 1;
                current = i;
                tasks[i].state = TASK_RUNNING;
                /* Set the stack pointer and jump to the task's entry point. */
                asm volatile("mv sp, %0" :: "r"(tasks[i].sp));
                asm volatile("jr %0" :: "r"(tasks[i].regs[0]));
                /* 
                 * When the task function returns, it will come back here.
                 * Mark the task as exited.
                 */
                tasks[i].state = TASK_EXITED;
            }
        }
        if (!any) break; /* No more ready tasks, exit the scheduler loop. */
    }
}