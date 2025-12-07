.section .start
    .globl _start

/* This is the entry point of the kernel. */
_start:
    /* Set up the initial stack pointer. */
    /* 'la' is a pseudo-instruction that loads the address of _stack_top into sp. */
    la sp, _stack_top

    /* Jump to the main kernel function in C. */
    call kmain

/* If kmain returns (which it shouldn't), enter an infinite loop. */
1:  
    wfi         /* Wait for an interrupt to save power. */
    j 1b        /* Jump back to the wfi instruction. */

    /* The .bss section is used for uninitialized data. */
    .section .bss
    .balign 16 /* Align the stack to a 16-byte boundary. */
_stack:
    .space 0x4000   /* Allocate 16 KB for the kernel stack. */
_stack_top:         /* Label marking the top of the stack. */