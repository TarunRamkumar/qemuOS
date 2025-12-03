    .section .start
    .globl _start
_start:
    /* Single-hart startup: just set stack and jump to kmain */

    la sp, _stack_top       /* set stack pointer */
    call kmain              /* call kernel main */

1:  wfi                     /* if kmain returns, idle */
    j 1b

    .section .bss
    .balign 16
_stack:
    .space 0x4000           /* 16 KB stack */
_stack_top:
