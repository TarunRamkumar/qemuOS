# Makefile for riscv teaching OS (with preemption, trap entry, context switch)
CROSS_PREFIX ?= riscv64-unknown-elf-
CC = $(CROSS_PREFIX)gcc
LD = $(CROSS_PREFIX)ld
OBJCOPY = $(CROSS_PREFIX)objcopy
QEMU ?= qemu-system-riscv64
# Explicitly include Zicsr/Zifencei since newer toolchains split these from the base ISA
CFLAGS = -march=rv64imac -mabi=lp64 -mcmodel=medany -ffreestanding -O0 -g -Wall -Wextra
LDFLAGS = -T link.ld
SRCDIR = src
BUILD = build
OBJS = $(BUILD)/start.o \
$(BUILD)/kernel.o \
$(BUILD)/uart.o \
$(BUILD)/trap.o \
$(BUILD)/trap_entry.o \
$(BUILD)/context_switch.o \
$(BUILD)/scheduler.o \
$(BUILD)/syscall.o \
$(BUILD)/timer.o \
$(BUILD)/fs.o \
$(BUILD)/shell.o \
$(BUILD)/user_programs.o \
$(BUILD)/string.o
all: check-toolchain $(BUILD)/kernel.elf
$(BUILD):
	mkdir -p $(BUILD)
$(BUILD)/%.o: $(SRCDIR)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/%.o: $(SRCDIR)/%.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/start.o: $(SRCDIR)/start.s | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
check-toolchain:
	@command -v $(CC) >/dev/null 2>&1 || { \
	        echo "Error: RISC-V GCC ($(CC)) not found."; \
	        echo "Please install a RISC-V cross toolchain or set CROSS_PREFIX to its prefix."; \
	        exit 1; \
	}
check-qemu:
	@command -v $(QEMU) >/dev/null 2>&1 || { \
	        echo "Error: QEMU system emulator ($(QEMU)) not found."; \
	        echo "Install qemu-system-riscv64 or point QEMU to the binary."; \
	        exit 1; \
	}
clean:
	rm -rf $(BUILD)
run: check-qemu all
	$(QEMU) -machine virt -nographic -bios default -kernel build/kernel.elf
.PHONY: all clean run
