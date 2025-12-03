# qemuOS

A mock operating system for RISC-V architecture with multi-tasking, I/O capabilities, and an interactive shell. This educational OS demonstrates core operating system concepts including task scheduling, system calls, trap handling, and context switching.

## Overview

qemuOS is a minimal operating system kernel designed to run on RISC-V hardware (or QEMU emulator). It provides:

- **Multi-tasking**: Cooperative task scheduling with support for up to 8 concurrent tasks
- **I/O System**: UART-based input/output via SBI (Supervisor Binary Interface) calls
- **System Calls**: User-space programs can interact with the kernel through system calls
- **File System**: Simple in-memory file system for storing program data
- **Interactive Shell**: Command-line interface for running programs and managing the system
- **Trap Handling**: Proper exception and interrupt handling with register preservation

## Architecture

### Target Platform
- **Architecture**: RISC-V 64-bit (RV64IMAC with Zicsr extension)
- **ABI**: LP64
- **Memory Model**: Medium-any code model
- **Boot Address**: 0x80200000

### Build System
The project uses a Makefile-based build system with RISC-V cross-compilation tools:
- Compiler: `riscv64-unknown-elf-gcc`
- Linker: `riscv64-unknown-elf-ld`
- Build artifacts are placed in the `build/` directory

## Components

### 1. Kernel (`kernel.c`)
The main kernel entry point that initializes all subsystems:
- Initializes UART for console I/O
- Sets up trap vector for exception handling
- Initializes the task scheduler
- Initializes timer (placeholder for future preemption)
- Spawns initial tasks: shell, hello program, and echo program
- Enters the scheduler to run tasks

### 2. UART I/O (`uart.c`, `uart.h`)
Provides console input/output capabilities using SBI calls:
- **`uart_init()`**: Initializes the UART (no-op, handled by SBI)
- **`uart_putc(char c)`**: Outputs a single character
- **`uart_puts(const char *s)`**: Outputs a null-terminated string (handles newline conversion)
- **`uart_getc_block()`**: Blocks until a character is received from input

Uses SBI calls:
- `SBI_CONSOLE_PUTCHAR` (1): Write character
- `SBI_CONSOLE_GETCHAR` (2): Read character

### 3. Trap Handling (`trap.c`, `trap.h`, `trap_entry.S`)
Handles exceptions and interrupts from user space:

**Trap Entry (`trap_entry.S`)**:
- Saves all 32 general-purpose registers plus `sepc` and `sstatus`
- Calls C trap handler with trap frame pointer
- Restores all registers on return

**Trap Handler (`trap.c`)**:
- Handles timer interrupts (code 5) - currently no-op
- Handles system call exceptions (codes 8, 9):
  - **SYS_YIELD** (1): Cooperative task yielding
  - **SYS_WRITE** (2): Write data to console
  - **SYS_SPAWN** (3): Create new task
- Updates `sepc` to advance past the `ecall` instruction
- Handles unhandled traps gracefully

### 4. Context Switching (`context_switch.S`)
Low-level assembly routine for saving and restoring task context:
- Saves all callee-saved and caller-saved registers from current task
- Restores all registers for the next task
- Each task has its own register save area (27 registers)

### 5. Task Scheduler (`scheduler.c`, `scheduler.h`)
Implements cooperative multi-tasking:

**Task States**:
- `TASK_EMPTY`: Unused task slot
- `TASK_READY`: Task ready to run
- `TASK_RUNNING`: Currently executing task
- `TASK_EXITED`: Task has completed

**Key Functions**:
- **`scheduler_init()`**: Initializes scheduler data structures
- **`scheduler_spawn(entry)`**: Creates a new task with:
  - 1KB stack per task
  - Entry point set in register 0 (ra)
  - Returns task ID (PID) or -1 on failure
- **`scheduler_yield()`**: Voluntarily yields CPU to next ready task
- **`scheduler_preempt()`**: Preempts current task (for future timer-based preemption)
- **`scheduler_yield_from_trap()`**: Yields from trap handler context
- **`scheduler_run()`**: Main scheduler loop that runs all ready tasks

**Limitations**:
- Maximum 8 concurrent tasks (`MAX_TASKS`)
- Round-robin scheduling among ready tasks
- Cooperative scheduling (tasks must yield voluntarily)

### 6. System Calls (`syscall.c`, `syscall.h`)
Provides kernel services to user programs:

**System Call Numbers**:
- `SYS_YIELD` (1): Yield CPU to another task
- `SYS_WRITE` (2): Write buffer to console
- `SYS_SPAWN` (3): Create new task

**Implementation**:
- **`do_sys_write(buf, len)`**: Writes data to UART console
- **`do_sys_yield()`**: Triggers task scheduler yield
- **`do_sys_spawn(entry)`**: Creates new task via scheduler

User programs invoke system calls using the `ecall` instruction with:
- `a7`: System call number
- `a0`, `a1`, etc.: Arguments
- Return value in `a0`

### 7. Timer (`timer.c`, `timer.h`)
Timer subsystem (placeholder for future preemption):
- **`timer_init()`**: Initializes timer (currently no-op)
- **`timer_handle_irq()`**: Handles timer interrupts (currently no-op)
- **`timer_now()`**: Returns current time (returns 0)

Future enhancement: Configure hardware timer for preemptive scheduling.

### 8. File System (`fs.c`, `fs.h`)
Simple in-memory file system:
- Stores program data as embedded strings
- **`fs_list_files(buf, maxlen)`**: Lists all available files
- **`fs_get_file_content(name, len)`**: Retrieves file content by name

**Available Files**:
- `hello`: Contains "Hello from embedded program!\n"
- `echo`: Contains "Echo program running.\n"

### 9. Shell (`shell.c`)
Interactive command-line interface:
- Reads input character-by-character from UART
- Supports line editing (80 character limit)
- **Commands**:
  - `ls`: List available files/programs
  - `run <name>`: Execute a program (e.g., `run hello`, `run echo`)
  - `help`: Display available commands

Runs as a persistent task that continuously reads and processes commands.

### 10. String Utilities (`string.c`, `string.h`)
Standard C string functions implemented for the kernel:
- **`memset(dst, c, n)`**: Fill memory with byte value
- **`memcpy(dst, src, n)`**: Copy memory block
- **`strcmp(a, b)`**: Compare two strings
- **`strncmp(a, b, n)`**: Compare strings up to n characters
- **`strlen(s)`**: Calculate string length

### 11. User Programs (`user_programs.c`)
Example user-space programs:
- **`user_prog_hello()`**: Reads and displays the "hello" file content
- **`user_prog_echo()`**: Reads and displays the "echo" file content

Both programs:
1. Read their content from the file system
2. Write to console using `SYS_WRITE`
3. Yield CPU using `SYS_YIELD`

### 12. Boot Code (`start.s`)
Assembly boot code that:
- Sets up the initial stack pointer (16KB stack)
- Jumps to `kmain()` function
- Enters idle loop (`wfi`) if kernel returns

### 13. Linker Script (`link.ld`)
Defines memory layout:
- Entry point: `_start`
- Base address: 0x80200000
- Sections: `.start`, `.text`, `.rodata`, `.data`, `.bss`
- Discards: `.comment`, `.note*`

## Building and Running

### Prerequisites
- RISC-V cross-compilation toolchain (`riscv64-unknown-elf-gcc`)
- QEMU with RISC-V support (`qemu-system-riscv64`)

### Build Commands
```bash
# Build the kernel
make

# Clean build artifacts
make clean

# Build and run in QEMU
make run
```

The `make run` command launches QEMU with:
- RISC-V virt machine
- No graphics (nographic mode)
- Default BIOS
- Kernel ELF as the boot image

### Running in QEMU
```bash
qemu-system-riscv64 -machine virt -nographic -bios default -kernel build/kernel.elf
```

## Usage

When the kernel boots, you'll see:
```
RISC-V Teaching Kernel starting (with preemption)...

Simple RISC-V Shell
> 
```

### Shell Commands
- **`ls`**: List available programs
- **`run hello`**: Execute the hello program
- **`run echo`**: Execute the echo program
- **`help`**: Show help message

### Example Session
```
> ls
hello
echo
> run hello
Hello from embedded program!
> run echo
Echo program running.
> help
Commands: ls, run <file>, help
>
```

## System Call Interface

User programs can make system calls using inline assembly:

```c
// Yield CPU
asm volatile("li a7, 1; ecall");

// Write to console
// a0 = buffer pointer, a1 = length
asm volatile("li a7, 2; ecall");

// Spawn new task
// a0 = entry point function pointer
// Returns PID in a0
asm volatile("li a7, 3; ecall");
```

## Memory Layout

- **Kernel Stack**: 16KB at boot (defined in `start.s`)
- **Task Stacks**: 1KB per task (8 tasks = 8KB total)
- **Code/Data**: Linked at 0x80200000
- **BSS**: Uninitialized data section

## Limitations and Future Enhancements

### Current Limitations
- Maximum 8 concurrent tasks
- Cooperative scheduling only (no preemption)
- Simple in-memory file system
- No memory protection or isolation
- No process management (tasks share address space)
- Timer subsystem not fully implemented

### Potential Enhancements
- Preemptive scheduling with timer interrupts
- Memory protection and virtual memory
- Process isolation
- More sophisticated file system
- Additional system calls (read, exit, etc.)
- Inter-process communication
- Dynamic memory allocation

## Code Structure

```
qemuOS/
├── src/
│   ├── kernel.c          # Main kernel entry point
│   ├── uart.c/h          # UART I/O subsystem
│   ├── trap.c/h          # Trap/exception handling
│   ├── trap_entry.S      # Trap entry assembly
│   ├── context_switch.S  # Context switching assembly
│   ├── scheduler.c/h     # Task scheduler
│   ├── syscall.c/h       # System call implementation
│   ├── timer.c/h         # Timer subsystem
│   ├── fs.c/h            # File system
│   ├── shell.c           # Interactive shell
│   ├── string.c/h        # String utilities
│   ├── user_programs.c   # Example user programs
│   └── start.s           # Boot code
├── build/                # Build artifacts
├── link.ld              # Linker script
├── Makefile             # Build configuration
└── README.md            # This file
```

## Educational Value

This OS demonstrates:
- **Low-level system programming**: Direct register manipulation, assembly code
- **OS fundamentals**: Task scheduling, context switching, system calls
- **RISC-V architecture**: CSR registers, trap handling, SBI interface
- **Embedded systems**: Minimal runtime, freestanding environment
- **Concurrency**: Multi-tasking without hardware memory protection

## License

This is an educational project for learning operating system concepts.
