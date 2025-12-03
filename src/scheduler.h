#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

typedef enum {
    TASK_EMPTY = 0,
    TASK_READY,
    TASK_RUNNING,
    TASK_EXITED
} task_state_t;

#define MAX_TASKS 8

typedef struct {
    uint64_t regs[27];
    uint64_t sp;
    void (*entry)(void);
    task_state_t state;
    uint8_t stack[1024];
} task_t;

void scheduler_init(void);
int scheduler_spawn(void (*entry)(void));
void scheduler_yield(void);
void scheduler_yield_from_trap(void);
void scheduler_preempt(void);
void scheduler_run(void);

#endif
