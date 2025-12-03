#include "timer.h"
#include <stdint.h>

/* For now, we don't use a hardware timer in S-mode with OpenSBI.
   We'll rely on cooperative scheduling only. */

uint64_t timer_now(void) {
    return 0;
}

void timer_init(void) {
    /* no-op for now */
}

void timer_handle_irq(void) {
    /* no-op: no timer interrupts configured */
}

