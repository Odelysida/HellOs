# HellOS - The Infernal Operating System
# Main Makefile

# Compiler settings (using system tools for now)
CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy

# Compiler flags
CFLAGS = -ffreestanding -O2 -Wall -Wextra -std=c11 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -fno-pic -m32
ASFLAGS = -f elf32
LDFLAGS = -nostdlib -m elf_i386

# Directories
BOOT_DIR = boot
KERNEL_DIR = kernel
DRIVERS_DIR = drivers
SHELL_DIR = shell
BUILD_DIR = build
TOOLS_DIR = tools

# Source files
BOOT_SOURCES = $(wildcard $(BOOT_DIR)/*.asm)
KERNEL_SOURCES = $(wildcard $(KERNEL_DIR)/*.c) $(wildcard $(KERNEL_DIR)/*.asm)
DRIVER_SOURCES = $(wildcard $(DRIVERS_DIR)/*/*.c) $(wildcard $(DRIVERS_DIR)/*/*.asm)
SHELL_SOURCES = $(wildcard $(SHELL_DIR)/*.c)

# Object files (bootloader is built directly)
# BOOT_OBJECTS = $(BOOT_SOURCES:$(BOOT_DIR)/%.asm=$(BUILD_DIR)/boot/%.o)
KERNEL_C_OBJECTS = $(filter %.c,$(KERNEL_SOURCES))
KERNEL_C_OBJECTS := $(KERNEL_C_OBJECTS:$(KERNEL_DIR)/%.c=$(BUILD_DIR)/kernel/%.o)
KERNEL_ASM_OBJECTS = $(filter %.asm,$(KERNEL_SOURCES))
KERNEL_ASM_OBJECTS := $(KERNEL_ASM_OBJECTS:$(KERNEL_DIR)/%.asm=$(BUILD_DIR)/kernel/%.o)
# Add kernel entry point
KERNEL_ASM_OBJECTS += $(BUILD_DIR)/kernel/kernel_entry.o
KERNEL_OBJECTS = $(KERNEL_C_OBJECTS) $(KERNEL_ASM_OBJECTS)
DRIVER_OBJECTS = $(patsubst $(DRIVERS_DIR)/%.c,$(BUILD_DIR)/drivers/%.o,$(DRIVER_SOURCES))
SHELL_OBJECTS = $(SHELL_SOURCES:$(SHELL_DIR)/%.c=$(BUILD_DIR)/shell/%.o)

# Output files
BOOTLOADER = $(BUILD_DIR)/hellboot.bin
KERNEL = $(BUILD_DIR)/hellos.bin
ISO_IMAGE = $(BUILD_DIR)/hellos.iso

# Default target
all: $(ISO_IMAGE)

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/boot $(BUILD_DIR)/kernel $(BUILD_DIR)/drivers $(BUILD_DIR)/shell

# Build bootloader
$(BOOTLOADER): $(BOOT_SOURCES) $(KERNEL) | $(BUILD_DIR)
	# Create unified bootloader binary with architectural fixes
	nasm -f bin $(BOOT_DIR)/unified_bootloader.asm -o $@

# Build kernel
$(KERNEL): $(KERNEL_OBJECTS) $(DRIVER_OBJECTS) $(SHELL_OBJECTS) | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -T $(KERNEL_DIR)/kernel.ld -o $(BUILD_DIR)/hellos.elf $^
	$(OBJCOPY) -O binary $(BUILD_DIR)/hellos.elf $@

# Debug build (with debug symbols and verbose output)
debug: CFLAGS += -g -DDEBUG -O0
debug: $(ISO_IMAGE)
	@echo "Debug build completed with full debug system integration"

# Build tools directory
tools: | $(BUILD_DIR)
	@mkdir -p $(TOOLS_DIR)
	@echo "Tools directory created"

# Hard disk image (alternative to ISO for testing)
hdd-image: $(BOOTLOADER) $(KERNEL) | $(BUILD_DIR)
	# Create 10MB hard disk image
	dd if=/dev/zero of=$(BUILD_DIR)/hellos.img bs=1024 count=10240
	# Install bootloader at sector 0
	dd if=$(BOOTLOADER) of=$(BUILD_DIR)/hellos.img bs=512 count=1 conv=notrunc
	# Install kernel starting at sector 2 (skip sector 1 for safety)
	dd if=$(KERNEL) of=$(BUILD_DIR)/hellos.img bs=512 seek=2 conv=notrunc
	@echo "Hard disk image created: $(BUILD_DIR)/hellos.img"
	@echo "Bootloader: sector 0, Kernel: sector 2+"

# Run hard disk image
run-hdd: hdd-image
	qemu-system-x86_64 -drive format=raw,file=$(BUILD_DIR)/hellos.img -m 512M -cpu qemu64 -serial file:$(BUILD_DIR)/serial.log -nographic

# Test hard disk image (10 second timeout)
test-hdd: hdd-image
	timeout 10s qemu-system-x86_64 -drive format=raw,file=$(BUILD_DIR)/hellos.img -m 512M -cpu qemu64 -serial file:$(BUILD_DIR)/serial.log -nographic || echo "HDD test completed"
	@echo "Serial output:"
	@cat $(BUILD_DIR)/serial.log

# Test with debug output enabled
test-debug: debug
	timeout 10s qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -cpu qemu64 -serial file:$(BUILD_DIR)/debug_serial.log -nographic || echo "Debug test completed"
	@echo "Debug serial output:"
	@cat $(BUILD_DIR)/debug_serial.log

# Build ISO image
$(ISO_IMAGE): $(BOOTLOADER) $(KERNEL) | $(BUILD_DIR)
	# Create ISO directory structure
	mkdir -p $(BUILD_DIR)/iso/boot
	# Combine bootloader and kernel into single boot image for El Torito
	cat $(BOOTLOADER) $(KERNEL) > $(BUILD_DIR)/iso/boot/hellboot.bin
	# Calculate proper boot load size (must be multiple of 512 bytes)
	boot_size=$$(stat -c %s $(BUILD_DIR)/iso/boot/hellboot.bin); \
	sectors=$$(( ($$boot_size + 511) / 512 )); \
	echo "Boot image size: $$boot_size bytes, $$sectors sectors"; \
	genisoimage -R -J -c boot/boot.cat -b boot/hellboot.bin -no-emul-boot -boot-load-size $$sectors -boot-info-table -o $(ISO_IMAGE) $(BUILD_DIR)/iso
	@echo "ISO created successfully: $(ISO_IMAGE)"
	@echo "Boot image contains bootloader + kernel: $$boot_size bytes"

# Compile C source files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble ASM source files  
$(BUILD_DIR)/%.o: %.asm | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# Specific rules for different directories
# Boot objects not needed - bootloader built directly
# $(BUILD_DIR)/boot/%.o: $(BOOT_DIR)/%.asm | $(BUILD_DIR)
# 	@mkdir -p $(dir $@)
# 	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/%.asm | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# Special rule for kernel entry point
$(BUILD_DIR)/kernel/kernel_entry.o: $(KERNEL_DIR)/kernel_entry.asm | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(AS) -f elf32 $< -o $@

$(BUILD_DIR)/drivers/%.o: $(DRIVERS_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Handle driver subdirectories
$(BUILD_DIR)/drivers/graphics/%.o: $(DRIVERS_DIR)/graphics/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/drivers/audio/%.o: $(DRIVERS_DIR)/audio/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/drivers/network/%.o: $(DRIVERS_DIR)/network/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/shell/%.o: $(SHELL_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Run in QEMU (WSL2 compatible - no KVM)
run: $(ISO_IMAGE)
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -cpu qemu64 -nographic

# Run in QEMU with serial output (better for WSL2)
run-serial: $(ISO_IMAGE)
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -cpu qemu64 -serial stdio -nographic

# Run in QEMU with GUI (requires X11 forwarding)
run-gui: $(ISO_IMAGE)
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -cpu qemu64 -display gtk

# Run in QEMU with KVM (for native Linux)
run-kvm: $(ISO_IMAGE)
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -enable-kvm -cpu host

# Run in QEMU with debugging
debug: $(ISO_IMAGE)
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -s -S -monitor stdio -cpu qemu64

# Run in QEMU with debugging and KVM
debug-kvm: $(ISO_IMAGE)
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -s -S -monitor stdio -enable-kvm -cpu host

# WSL-specific debugging targets
# Debug with serial output (WSL2 friendly)
debug-wsl: $(ISO_IMAGE)
	@echo "Starting HellOS in debug mode (WSL2 compatible)"
	@echo "GDB will listen on localhost:1234"
	@echo "QEMU monitor available in this terminal"
	@echo "In another terminal, run: gdb -ex 'target remote localhost:1234'"
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -cpu qemu64 -nographic -s -S

# Debug with VNC display (accessible from Windows)
debug-wsl-vnc: $(ISO_IMAGE)
	@echo "Starting HellOS in debug mode with VNC display"
	@echo "GDB will listen on localhost:1234"
	@echo "VNC display available at localhost:5900"
	@echo "In another terminal, run: gdb -ex 'target remote localhost:1234'"
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -cpu qemu64 -vnc :0 -s -S

# Debug with telnet monitor (easier for WSL)
debug-wsl-telnet: $(ISO_IMAGE)
	@echo "Starting HellOS in debug mode with telnet monitor"
	@echo "GDB will listen on localhost:1234"
	@echo "QEMU monitor available via: telnet localhost 55555"
	@echo "Serial output will be written to $(BUILD_DIR)/serial.log"
	@echo "In another terminal, run: gdb -ex 'target remote localhost:1234'"
	@mkdir -p $(BUILD_DIR)
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -cpu qemu64 -nographic -serial file:$(BUILD_DIR)/serial.log -monitor telnet:localhost:55555,server,nowait -s -S

# Debug with serial output to file (clean separation)
debug-wsl-serial: $(ISO_IMAGE)
	@echo "Starting HellOS in debug mode with serial logging"
	@echo "GDB will listen on localhost:1234"
	@echo "QEMU monitor available in this terminal"
	@echo "Serial output will be written to $(BUILD_DIR)/serial.log"
	@echo "In another terminal, run: gdb -ex 'target remote localhost:1234'"
	@echo "Monitor serial output with: tail -f $(BUILD_DIR)/serial.log"
	@mkdir -p $(BUILD_DIR)
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -cpu qemu64 -nographic -serial file:$(BUILD_DIR)/serial.log -s -S

# Debug with logging (useful for WSL troubleshooting)
debug-wsl-log: $(ISO_IMAGE)
	@echo "Starting HellOS in debug mode with extensive logging"
	@echo "GDB will listen on localhost:1234"
	@echo "Logs will be written to $(BUILD_DIR)/debug.log"
	@echo "Serial output will be written to $(BUILD_DIR)/serial.log"
	@mkdir -p $(BUILD_DIR)
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -cpu qemu64 -nographic -serial file:$(BUILD_DIR)/serial.log -s -S \
		-d int,cpu_reset,guest_errors -D $(BUILD_DIR)/debug.log

# Create GDB script for easier debugging
debug-gdb-script: | $(BUILD_DIR)
	@echo "Creating GDB debugging script..."
	@echo "# HellOS GDB Debugging Script" > $(BUILD_DIR)/debug.gdb
	@echo "target remote localhost:1234" >> $(BUILD_DIR)/debug.gdb
	@echo "set architecture i386" >> $(BUILD_DIR)/debug.gdb
	@echo "# Break at kernel entry" >> $(BUILD_DIR)/debug.gdb
	@echo "break *0x100000" >> $(BUILD_DIR)/debug.gdb
	@echo "# Break at bootloader start" >> $(BUILD_DIR)/debug.gdb
	@echo "break *0x7c00" >> $(BUILD_DIR)/debug.gdb
	@echo "# Display registers" >> $(BUILD_DIR)/debug.gdb
	@echo "define regs" >> $(BUILD_DIR)/debug.gdb
	@echo "    info registers" >> $(BUILD_DIR)/debug.gdb
	@echo "    info registers eflags" >> $(BUILD_DIR)/debug.gdb
	@echo "end" >> $(BUILD_DIR)/debug.gdb
	@echo "# Continue execution" >> $(BUILD_DIR)/debug.gdb
	@echo "continue" >> $(BUILD_DIR)/debug.gdb
	@echo "GDB script created: $(BUILD_DIR)/debug.gdb"
	@echo "Usage: gdb -x $(BUILD_DIR)/debug.gdb"

# Start debugging session with automatic GDB connection
debug-wsl-auto: debug-gdb-script $(ISO_IMAGE)
	@echo "Starting automated debugging session..."
	@echo "Starting QEMU in background..."
	@mkdir -p $(BUILD_DIR)
	@qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -cpu qemu64 -nographic -serial file:$(BUILD_DIR)/serial.log -s -S &
	@echo "Waiting for QEMU to start..."
	@sleep 2
	@echo "Starting GDB..."
	@echo "Serial output available at: $(BUILD_DIR)/serial.log"
	@gdb -x $(BUILD_DIR)/debug.gdb || echo "GDB session ended"

# Quick test run (boots and exits after 10 seconds)
test: $(ISO_IMAGE)
	timeout 10s qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -cpu qemu64 -nographic || echo "HellOS test completed"

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Install cross-compiler tools
install-tools:
	@echo "Installing cross-compiler tools..."
	@echo "Please run: sudo apt-get install gcc-multilib nasm genisoimage qemu-system-x86"
	@echo "Then build cross-compiler with: make build-cross-compiler"

# Build cross-compiler (simplified)
build-cross-compiler:
	cd $(TOOLS_DIR) && ./build-cross-compiler.sh

# Help
help:
	@echo "HellOS Build System"
	@echo "=================="
	@echo "Targets:"
	@echo "  all              - Build complete HellOS ISO"
	@echo "  bootloader       - Build bootloader only"
	@echo "  kernel           - Build kernel only"
	@echo "  run              - Run HellOS in QEMU (WSL2 compatible, text mode)"
	@echo "  run-gui          - Run HellOS in QEMU with GUI (requires X11)"
	@echo "  run-kvm          - Run HellOS in QEMU with KVM (native Linux)"
	@echo "  test             - Quick test run (10 seconds)"
	@echo "  hdd-image        - Build hard disk image with test kernel"
	@echo "  minimal-hdd      - Build hard disk image with minimal kernel"
	@echo "  run-hdd          - Run hard disk image"
	@echo "  run-minimal      - Run minimal hard disk image (RECOMMENDED)"
	@echo "  test-hdd         - Test hard disk image (10 seconds)"
	@echo "  test-minimal     - Test minimal hard disk image (10 seconds)"
	@echo "  debug            - Run HellOS in QEMU with debugging"
	@echo "  debug-kvm        - Run HellOS in QEMU with debugging and KVM"
	@echo "  debug-wsl        - Debug HellOS in WSL2 (monitor only)"
	@echo "  debug-wsl-serial - Debug HellOS in WSL2 with serial logging"
	@echo "  debug-wsl-vnc    - Debug HellOS in WSL2 with VNC display"
	@echo "  debug-wsl-telnet - Debug HellOS in WSL2 with telnet monitor"
	@echo "  debug-wsl-log    - Debug HellOS in WSL2 with extensive logging"
	@echo "  debug-wsl-auto   - Automated debugging session with GDB"
	@echo "  debug-gdb-script - Create GDB debugging script"
	@echo "  clean            - Clean build artifacts"
	@echo "  install-tools    - Install required tools"
	@echo "  help             - Show this help"

.PHONY: all run run-gui run-kvm test debug debug-kvm debug-wsl debug-wsl-serial debug-wsl-vnc debug-wsl-telnet debug-wsl-log debug-wsl-auto debug-gdb-script clean install-tools build-cross-compiler help hdd-image minimal-hdd run-hdd run-minimal test-hdd test-minimal minimal-kernel 