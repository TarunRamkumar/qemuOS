# qemuOS

A mock operating system for RISC-V architecture with multi-tasking, I/O capabilities, and an interactive shell. This educational OS demonstrates core operating system concepts including task scheduling, system calls, trap handling, and context switching.

## Overview

qemuOS is a minimal operating system kernel designed to run on RISC-V hardware (or QEMU emulator). It provides:

- **Multi-tasking**: Cooperative task scheduling with support for up to 8 concurrent tasks
- **I/O System**: UART-based input/output via SBI (Supervisor Binary Interface) calls
- **System Calls**: User-space programs can interact with the kernel through system calls
- **File System**: Full-featured in-memory file system with file descriptors, read/write operations, and file management
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
  - **SYS_OPEN** (4): Open file
  - **SYS_READ** (5): Read from file
  - **SYS_WRITE_FD** (6): Write to file
  - **SYS_CLOSE** (7): Close file descriptor
  - **SYS_CREATE** (8): Create new file
  - **SYS_DELETE** (9): Delete file
  - **SYS_SEEK** (10): Seek in file
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
- `SYS_OPEN` (4): Open file with read/write flags
- `SYS_READ` (5): Read from file descriptor
- `SYS_WRITE_FD` (6): Write to file descriptor
- `SYS_CLOSE` (7): Close file descriptor
- `SYS_CREATE` (8): Create new file
- `SYS_DELETE` (9): Delete file
- `SYS_SEEK` (10): Seek to position in file

**Implementation**:
- **`do_sys_write(buf, len)`**: Writes data to UART console
- **`do_sys_yield()`**: Triggers task scheduler yield
- **`do_sys_spawn(entry)`**: Creates new task via scheduler
- **`do_sys_open(name, flags)`**: Opens a file, returns file descriptor
- **`do_sys_read(fd, buf, len)`**: Reads from file descriptor
- **`do_sys_write_fd(fd, buf, len)`**: Writes to file descriptor
- **`do_sys_close(fd)`**: Closes file descriptor
- **`do_sys_create(name)`**: Creates a new empty file
- **`do_sys_delete(name)`**: Deletes a file
- **`do_sys_seek(fd, offset)`**: Seeks to position in file

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
Full-featured in-memory file system with file descriptor support:

**File System Features**:
- File descriptor-based I/O (FDs start at 3, 0-2 reserved)
- Support for up to 16 files and 16 open file descriptors
- Read and write operations with position tracking
- File creation and deletion
- Embedded read-only files for initial program data

**File System Limits**:
- Maximum 16 files
- Maximum 16 open file descriptors
- 4KB maximum file size per file
- 32 character filename limit

**Key Functions**:
- **`fs_init()`**: Initializes file system and creates initial embedded files
- **`fs_create(name)`**: Creates a new empty file
- **`fs_delete(name)`**: Deletes a file
- **`fs_open(name, flags)`**: Opens a file, returns file descriptor
  - Flags: `FD_READ` (0x1) for reading, `FD_WRITE` (0x2) for writing
- **`fs_read(fd, buf, len)`**: Reads data from file descriptor
- **`fs_write(fd, buf, len)`**: Writes data to file descriptor
- **`fs_close(fd)`**: Closes file descriptor
- **`fs_seek(fd, offset)`**: Seeks to position in file
- **`fs_list_files(buf, maxlen)`**: Lists all files with their sizes
- **`fs_get_file_size(name)`**: Gets file size by name
- **`fs_get_file_content(name, len)`**: Legacy function for backward compatibility

**Initial Embedded Files** (read-only):
- `hello`: Contains "Hello from embedded program!\n"
- `echo`: Contains "Echo program running.\n"

**Memory Management**:
- Each file gets a dedicated 4KB buffer from a static pool
- Embedded files point to read-only data in the binary
- Created files use allocated writable buffers
- Embedded files cannot be written to (read-only protection)

### 9. Shell (`shell.c`)
Interactive command-line interface:
- Reads input character-by-character from UART
- Supports line editing (80 character limit)
- **Commands**:
  - `ls`: List all files with their sizes
  - `cat <file>`: Display file contents
  - `create <file>`: Create a new empty file
  - `delete <file>`: Delete a file
  - `write <file> <text>`: Write text to a file
  - `run <name>`: Execute a program (e.g., `run hello`, `run echo`, `run fstest`)
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
- **`user_prog_hello()`**: Reads and displays the "hello" file content (uses legacy API)
- **`user_prog_echo()`**: Reads and displays the "echo" file content (uses legacy API)
- **`user_prog_fstest()`**: Demonstrates file descriptor API usage

Programs demonstrate:
1. Reading from file system (both legacy and file descriptor APIs)
2. Writing to console using `SYS_WRITE`
3. File operations using system calls
4. Yielding CPU using `SYS_YIELD`

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
- **`ls`**: List all files with sizes
- **`cat <file>`**: Display file contents
- **`create <file>`**: Create a new empty file
- **`delete <file>`**: Delete a file
- **`write <file> <text>`**: Write text to a file
- **`run <prog>`**: Execute a program (`hello`, `echo`, `fstest`)
- **`help`**: Show help message

### Example Session
```
> ls
hello (29)
echo (22)
> cat hello
Hello from embedded program!
> create test
File created
> write test Hello World!
Written to file
> cat test
Hello World!
> run fstest
File system test program
Read from file: Hello from embedded program!
> delete test
File deleted
> ls
hello (29)
echo (22)
> help
Commands:
  ls              - List files
  cat <file>      - Display file contents
  create <file>   - Create new file
  delete <file>   - Delete file
  write <file> <text> - Write text to file
  run <prog>      - Run program
  help            - Show this help
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

// Open file
// a0 = filename (char*), a1 = flags (FD_READ=0x1, FD_WRITE=0x2)
// Returns file descriptor in a0 (or -1 on error)
asm volatile("li a7, 4; ecall");

// Read from file
// a0 = file descriptor, a1 = buffer (char*), a2 = length
// Returns bytes read in a0
asm volatile("li a7, 5; ecall");

// Write to file
// a0 = file descriptor, a1 = buffer (char*), a2 = length
// Returns bytes written in a0
asm volatile("li a7, 6; ecall");

// Close file
// a0 = file descriptor
// Returns 0 on success, -1 on error in a0
asm volatile("li a7, 7; ecall");

// Create file
// a0 = filename (char*)
// Returns 0 on success, -1 on error in a0
asm volatile("li a7, 8; ecall");

// Delete file
// a0 = filename (char*)
// Returns 0 on success, -1 on error in a0
asm volatile("li a7, 9; ecall");

// Seek in file
// a0 = file descriptor, a1 = offset
// Returns new position in a0
asm volatile("li a7, 10; ecall");
```

## Using the File System

The file system provides both shell commands and system call interfaces for file operations.

### Using the File System from the Shell

The shell provides convenient commands for file operations:

**List Files**:
```
> ls
hello (29)
echo (22)
test (13)
```

**View File Contents**:
```
> cat hello
Hello from embedded program!
```

**Create a New File**:
```
> create myfile
File created
```

**Write to a File**:
```
> write myfile This is my file content
Written to file
```

**Read from a File**:
```
> cat myfile
This is my file content
```

**Delete a File**:
```
> delete myfile
File deleted
```

### Using the File System from User Programs

User programs can interact with the file system through system calls. Here's a complete example:

```c
#include "syscall.h"
#include <stdint.h>

#define FD_READ 0x1
#define FD_WRITE 0x2

// Helper function to make system calls
static inline int syscall(int num, uint64_t a0, uint64_t a1, uint64_t a2) {
    register long a7 asm("a7") = num;
    register long a0_reg asm("a0") = a0;
    register long a1_reg asm("a1") = a1;
    register long a2_reg asm("a2") = a2;
    asm volatile("ecall" : "+r"(a0_reg) : "r"(a7), "r"(a1_reg), "r"(a2_reg) : "memory");
    return (int)a0_reg;
}

void my_program(void) {
    // Create a new file
    int result = syscall(8, (uint64_t)"output", 0, 0);  // SYS_CREATE
    if (result != 0) {
        // Handle error
        return;
    }
    
    // Open file for writing
    int fd = syscall(4, (uint64_t)"output", FD_WRITE, 0);  // SYS_OPEN
    if (fd < 0) {
        // Handle error
        return;
    }
    
    // Write data to file
    const char *data = "Hello from program!\n";
    int len = 20;
    int written = syscall(6, fd, (uint64_t)data, len);  // SYS_WRITE_FD
    
    // Close file
    syscall(7, fd, 0, 0);  // SYS_CLOSE
    
    // Open file for reading
    fd = syscall(4, (uint64_t)"output", FD_READ, 0);  // SYS_OPEN
    if (fd >= 0) {
        char buf[256];
        int n = syscall(5, fd, (uint64_t)buf, 255);  // SYS_READ
        if (n > 0) {
            buf[n] = 0;
            // Use the data...
        }
        syscall(7, fd, 0, 0);  // SYS_CLOSE
    }
    
    // Yield CPU
    do_sys_yield();
}
```

### File System API Reference

**Opening Files**:
- Use `SYS_OPEN` (4) with filename and flags
- Flags: `FD_READ` (0x1) for reading, `FD_WRITE` (0x2) for writing
- Returns file descriptor (>= 3) or -1 on error
- File descriptors 0, 1, 2 are reserved (stdin, stdout, stderr)

**Reading from Files**:
- Use `SYS_READ` (5) with file descriptor, buffer, and length
- Returns number of bytes read (0 at EOF, -1 on error)
- Automatically advances file position

**Writing to Files**:
- Use `SYS_WRITE_FD` (6) with file descriptor, buffer, and length
- Returns number of bytes written or -1 on error
- Automatically advances file position
- Cannot write to embedded (read-only) files

**Seeking in Files**:
- Use `SYS_SEEK` (10) with file descriptor and offset
- Returns new position or -1 on error
- Position is clamped to valid range [0, file_size]

**File Management**:
- `SYS_CREATE` (8): Creates empty file, returns 0 on success
- `SYS_DELETE` (9): Deletes file, returns 0 on success
- `SYS_CLOSE` (7): Closes file descriptor, returns 0 on success

### File System Best Practices

1. **Always close file descriptors**: Failing to close FDs wastes resources
2. **Check return values**: System calls return -1 on error
3. **Handle embedded files**: Embedded files are read-only; create new files for writing
4. **Respect file limits**: Maximum 16 files and 16 open FDs
5. **File size limits**: Each file can hold up to 4KB of data
6. **Filename length**: Keep filenames under 32 characters

### File System Internals

- **Memory Layout**: Each file gets a dedicated 4KB buffer from a static pool
- **File Types**: 
  - Embedded files: Point to read-only data in the binary
  - Created files: Use allocated writable buffers
- **File Descriptors**: Map to file entries with position and flags
- **Position Tracking**: Each open FD maintains its own read/write position

## Memory Layout

- **Kernel Stack**: 16KB at boot (defined in `start.s`)
- **Task Stacks**: 1KB per task (8 tasks = 8KB total)
- **Code/Data**: Linked at 0x80200000
- **BSS**: Uninitialized data section

## Limitations and Future Enhancements

### Current Limitations
- Maximum 8 concurrent tasks
- Cooperative scheduling only (no preemption)
- In-memory file system (data lost on reboot)
- Maximum 16 files and 16 open file descriptors
- 4KB maximum file size
- No memory protection or isolation
- No process management (tasks share address space)
- Timer subsystem not fully implemented

### Potential Enhancements
- Preemptive scheduling with timer interrupts
- Memory protection and virtual memory
- Process isolation
- Persistent file system (disk storage)
- Directory support and hierarchical file structure
- File permissions and access control
- Additional system calls (read from stdin, exit, etc.)
- Inter-process communication
- Dynamic memory allocation
- Larger file size limits

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
