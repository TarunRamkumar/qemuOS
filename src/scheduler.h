#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

/* Defines the possible states of a task. */
typedef enum {
    TASK_EMPTY = 0,     /* Task slot is available. */
    TASK_READY,         /* Task is ready to run. */
    TASK_RUNNING,       /* Task is currently running. */
    TASK_EXITED         /* Task has finished execution. */
} task_state_t;

/* Maximum number of tasks the scheduler can manage. */
#define MAX_TASKS 8

/* Task Control Block (TCB) structure. */
typedef struct {
    uint64_t regs[27];      /* Saved registers. */
    uint64_t sp;            /* Saved stack pointer. */
    void (*entry)(void);    /* Entry point of the task function. */
    task_state_t state;     /* Current state of the task. */
    uint8_t stack[1024];  /* The task's own stack. */
} task_t;

/* Initializes the scheduler. */
void scheduler_init(void);
/* Spawns a new task. */
int scheduler_spawn(void (*entry)(void));
/* Yields the CPU to another task cooperatively. */
void scheduler_yield(void);
/* Yields the CPU from a trap handler. */
void scheduler_yield_from_trap(void);
/* Preempts the current task (for preemptive multitasking). */
void scheduler_preempt(void);
/* Starts the scheduler to run the tasks. */
void scheduler_run(void);

#endif