# HellOS Development Tools Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = 

# Tools to build
TOOLS = debug_viewer memory_analyzer boot_checker

# Default target
all: $(TOOLS)

# Debug log viewer
debug_viewer: debug_viewer.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Memory analyzer (placeholder)
memory_analyzer: memory_analyzer.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Boot checker (placeholder)
boot_checker: boot_checker.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Clean build artifacts
clean:
	rm -f $(TOOLS)

# Install tools to system
install: $(TOOLS)
	mkdir -p /usr/local/bin
	cp $(TOOLS) /usr/local/bin/

# Uninstall tools
uninstall:
	rm -f /usr/local/bin/debug_viewer
	rm -f /usr/local/bin/memory_analyzer
	rm -f /usr/local/bin/boot_checker

# Help
help:
	@echo "HellOS Development Tools"
	@echo "Available targets:"
	@echo "  all          - Build all tools"
	@echo "  debug_viewer - Build debug log viewer"
	@echo "  clean        - Remove build artifacts"
	@echo "  install      - Install tools to /usr/local/bin"
	@echo "  uninstall    - Remove installed tools"
	@echo "  help         - Show this help"

.PHONY: all clean install uninstall help 