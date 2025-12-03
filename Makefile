# Makefile for riscv teaching OS (with preemption, trap entry, context switch)
CROSS = riscv64-unknown-elf-
CC = $(CROSS)gcc
LD = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy
CFLAGS = -march=rv64imac_zicsr -mabi=lp64 -mcmodel=medany -ffreestanding -O2 -g -Wall -Wextra
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

all: $(BUILD)/kernel.elf

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

clean:
	rm -rf $(BUILD)

run: all
	qemu-system-riscv64 -machine virt -nographic -bios default -kernel build/kernel.elf

.PHONY: all clean run
