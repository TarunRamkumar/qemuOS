# context_switch.S
# This file contains the implementation of the context switch routine for the RISC-V OS.
# A context switch is the process of storing the state of a process or thread so that it can be restored and resume execution at a later point. 
# This allows multiple processes to share a single CPU.

.section .text
.global context_switch
.type context_switch, @function

# void context_switch(uint64_t* old_context, uint64_t* new_context);
#
# Arguments:
# a0: pointer to the context of the currently running process (old_context)
# a1: pointer to the context of the next process to run (new_context)
#
# The context is a structure that holds the saved registers of a process.
# This function saves the registers of the current process into its context,
# and then loads the registers of the new process from its context.
#
context_switch:
    # Move arguments to temporary registers, as a0 and a1 will be overwritten
    # when restoring the context of the new process.
    mv t0, a0  # t0 = old_context
    mv t1, a1  # t1 = new_context

    # Save the registers of the current process into its context (old_context).
    # The order of saved registers defines the structure of the context.
    # The offsets are in bytes.
    sd ra, 0(t0)      # Return Address
    sd t0, 8(t0)      # Temporary register 0
    sd t1, 16(t0)     # Temporary register 1
    sd t2, 24(t0)     # Temporary register 2
    sd s0, 32(t0)     # Saved register 0 / Frame pointer
    sd s1, 40(t0)     # Saved register 1
    sd a0, 48(t0)     # Argument register 0
    sd a1, 56(t0)     # Argument register 1
    sd a2, 64(t0)     # Argument register 2
    sd a3, 72(t0)     # Argument register 3
    sd a4, 80(t0)     # Argument register 4
    sd a5, 88(t0)     # Argument register 5
    sd a6, 96(t0)     # Argument register 6
    sd a7, 104(t0)    # Argument register 7
    sd s2, 112(t0)    # Saved register 2
    sd s3, 120(t0)    # Saved register 3
    sd s4, 128(t0)    # Saved register 4
    sd s5, 136(t0)    # Saved register 5
    sd s6, 144(t0)    # Saved register 6
    sd s7, 152(t0)    # Saved register 7
    sd s8, 160(t0)    # Saved register 8
    sd s9, 168(t0)    # Saved register 9
    sd s10, 176(t0)   # Saved register 10
    sd s11, 184(t0)   # Saved register 11
    sd t3, 192(t0)    # Temporary register 3
    sd t4, 200(t0)    # Temporary register 4
    sd t5, 208(t0)    # Temporary register 5
    sd t6, 216(t0)    # Temporary register 6

    # Load the registers of the new process from its context (new_context).
    # This will overwrite the current register values.
    ld ra, 0(t1)      # Return Address
    ld t0, 8(t1)      # Temporary register 0
    ld t1, 16(t1)     # Temporary register 1
    ld t2, 24(t1)     # Temporary register 2
    ld s0, 32(t1)     # Saved register 0 / Frame pointer
    ld s1, 40(t1)     # Saved register 1
    ld a0, 48(t1)     # Argument register 0
    ld a1, 56(t1)     # Argument register 1
    ld a2, 64(t1)     # Argument register 2
    ld a3, 72(t1)     # Argument register 3
    ld a4, 80(t1)     # Argument register 4
    ld a5, 88(t1)     # Argument register 5
    ld a6, 96(t1)     # Argument register 6
    ld a7, 104(t1)    # Argument register 7
    ld s2, 112(t1)    # Saved register 2
    ld s3, 120(t1)    # Saved register 3
    ld s4, 128(t1)    # Saved register 4
    ld s5, 136(t1)    # Saved register 5
    ld s6, 144(t1)    # Saved register 6
    ld s7, 152(t1)    # Saved register 7
    ld s8, 160(t1)    # Saved register 8
    ld s9, 168(t1)    # Saved register 9
    ld s10, 176(t1)   # Saved register 10
    ld s11, 184(t1)   # Saved register 11
    ld t3, 192(t1)    # Temporary register 3
    ld t4, 200(t1)    # Temporary register 4
    ld t5, 208(t1)    # Temporary register 5
    ld t6, 216(t1)    # Temporary register 6

    # Return to the address that was loaded into the 'ra' register from the new context.
    # This will resume execution of the new process.
    ret