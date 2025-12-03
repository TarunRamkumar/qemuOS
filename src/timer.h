#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timer_init(void);
void timer_handle_irq(void);
uint64_t timer_now(void);

#endif
