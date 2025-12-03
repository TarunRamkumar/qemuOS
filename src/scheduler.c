#include "scheduler.h"
#include "uart.h"
#include "string.h"

extern void context_switch(uint64_t*, uint64_t*);

static task_t tasks[MAX_TASKS];
static int current = -1;

void scheduler_init(void) {
    memset(tasks, 0, sizeof(tasks));
    current = -1;
}

static void zero_regs(uint64_t *r) {
    for (int i = 0; i < 27; i++) r[i] = 0;
}

int scheduler_spawn(void (*entry)(void)) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_EMPTY || tasks[i].state == TASK_EXITED) {
            tasks[i].state = TASK_READY;
            tasks[i].entry = entry;
            tasks[i].sp = (uint64_t)&tasks[i].stack[1024];
            zero_regs(tasks[i].regs);
            tasks[i].regs[0] = (uint64_t)entry;
            return i;
        }
    }
    return -1;
}

static int next_ready(int s) {
    for (int i = 1; i <= MAX_TASKS; i++) {
        int j = (s + i) % MAX_TASKS;
        if (tasks[j].state == TASK_READY) return j;
    }
    return -1;
}

void scheduler_yield(void) {
    int prev = current;
    int nxt = next_ready(current);
    if (nxt == -1) return;
    if (prev == -1) {
        current = nxt;
        tasks[nxt].state = TASK_RUNNING;
        asm volatile("mv sp, %0" :: "r"(tasks[nxt].sp));
        asm volatile("jr %0" :: "r"(tasks[nxt].regs[0]));
        return;
    }
    uint64_t sp;
    asm volatile("mv %0, sp" : "=r"(sp));
    tasks[prev].sp = sp;
    current = nxt;
    tasks[nxt].state = TASK_RUNNING;
    asm volatile("mv sp, %0" :: "r"(tasks[nxt].sp));
    asm volatile("jr %0" :: "r"(tasks[nxt].regs[0]));
}

void scheduler_preempt(void) {
    int prev = current;
    int nxt = next_ready(current);
    if (nxt == -1) return;
    if (prev == nxt) return;

    uint64_t sp;
    asm volatile("mv %0, sp" : "=r"(sp));
    if (prev >= 0) tasks[prev].sp = sp;

    current = nxt;
    tasks[nxt].state = TASK_RUNNING;
    asm volatile("mv sp, %0" :: "r"(tasks[nxt].sp));
    asm volatile("jr %0" :: "r"(tasks[nxt].regs[0]));
}

void scheduler_yield_from_trap(void) {
    scheduler_preempt();
}

void scheduler_run(void) {
    while (1) {
        int any = 0;
        for (int i = 0; i < MAX_TASKS; i++) {
            if (tasks[i].state == TASK_READY) {
                any = 1;
                current = i;
                tasks[i].state = TASK_RUNNING;
                asm volatile("mv sp, %0" :: "r"(tasks[i].sp));
                asm volatile("jr %0" :: "r"(tasks[i].regs[0]));
                tasks[i].state = TASK_EXITED;
            }
        }
        if (!any) break;
    }
}
