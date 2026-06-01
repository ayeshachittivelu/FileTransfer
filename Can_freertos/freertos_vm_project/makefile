# =========================================================
# FreeRTOS AArch64 Cortex-A55 EL1 (Virtual GIC/Timer)
# =========================================================

CROSS   = aarch64-linux-gnu-
CC      = $(CROSS)gcc
LD      = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy

CPU     = -mcpu=cortex-a55

# Added -fno-stack-protector to avoid extra dependencies
CFLAGS  = -O2 -ffreestanding -nostdlib -fno-stack-protector \
          -Wall \
          -Wno-pointer-to-int-cast \
          -Wno-int-to-pointer-cast \
          $(CPU) \
          -I app \
          -I portable \
          -I kernel/FreeRTOS-Kernel/include

# FIX: Added -static to prevent dynamic linking 
# FIX: Added --build-id=none to remove Linux-specific ELF notes
LDFLAGS = -T linker/freertos.ld -nostdlib -static -Wl,--build-id=none

# ---------------------------------------------------------

APP_SRC = \
    app/startup.s \
    app/main.c \
    app/libc_stubs.c

PORT_SRC = \
    portable/port.c \
    portable/portASM.S \
    portable/MemMang/heap_4.c

KERNEL_SRC = \
    kernel/FreeRTOS-Kernel/tasks.c \
    kernel/FreeRTOS-Kernel/list.c \
    kernel/FreeRTOS-Kernel/queue.c \
    kernel/FreeRTOS-Kernel/timers.c \
    kernel/FreeRTOS-Kernel/event_groups.c \
    kernel/FreeRTOS-Kernel/stream_buffer.c

SRCS = $(APP_SRC) $(PORT_SRC) $(KERNEL_SRC)

OBJS = $(patsubst %.c,build/%.o,$(filter %.c,$(SRCS))) \
       $(patsubst %.s,build/%.o,$(filter %.s,$(SRCS))) \
       $(patsubst %.S,build/%.o,$(filter %.S,$(SRCS)))

# ---------------------------------------------------------

all: directories bin/freertos.elf bin/freertos.bin

directories:
	mkdir -p build/app
	mkdir -p build/portable
	mkdir -p build/portable/MemMang
	mkdir -p build/kernel/FreeRTOS-Kernel
	mkdir -p bin

# Compile C
build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile Assembly (.s and .S)
build/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link
# FIX: Ensure $(CPU) and $(LDFLAGS) are used correctly with the CC driver
bin/freertos.elf: $(OBJS)
	$(CC) $(CPU) $(OBJS) $(LDFLAGS) -o $@

# Create binary
bin/freertos.bin: bin/freertos.elf
	$(OBJCOPY) -O binary $< $@

# Clean
clean:
	rm -rf build bin

.PHONY: all clean directories
