#ifndef TRAP_H
#define TRAP_H

#include <stdint.h>

void handle_trap_from_asm(uint64_t *tf);

#endif
