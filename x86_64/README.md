# HellOS - The Infernal Operating System 🔥

A modern x86_64 operating system with a hellish theme, inspired by TempleOS but designed for the digital underworld.

## Features

- **Architecture**: x86_64 (64-bit) with UEFI boot
- **Graphics**: 680x480 resolution with 32-color hellish palette
- **Audio**: 3-channel audio system (Square, Sine, Sawtooth waveforms)
- **Network**: TCP/UDP/Socket support (planned)
- **Shell**: Infernal Shell (InfSh) with demonic commands
- **Window Manager**: Pandemonium WM with flame effects
- **Theme**: Complete hellish aesthetic with fire, demons, and infernal terminology

## Quick Start

### Prerequisites

Install the required tools:
```bash
sudo apt-get install gcc-multilib nasm genisoimage qemu-system-x86
```

### Building

```bash
# Build the complete OS
make

# Run in QEMU
make run

# Debug mode
make debug
```

## System Architecture

### Boot Process
1. **UEFI Bootloader** - Initializes graphics and loads kernel
2. **Kernel Init** - Sets up memory management and interrupts
3. **Driver Loading** - Loads graphics, audio, and network drivers
4. **Shell Launch** - Starts the Infernal Shell

### Infernal Shell Commands

- `summon <program>` - Execute a soul (program)
- `banish <pid>` - Terminate a demon (process)
- `scry [path]` - List contents of a realm (directory)
- `conjure <file>` - Create a new artifact (file)
- `purge <file>` - Delete an artifact (file)
- `realm [path]` - Change current realm (directory)
- `souls` - List active souls (processes)
- `demons` - List system demons (system processes)
- `inferno` - System information
- `help` - Show available incantations
- `about` - About HellOS

### File System Structure

```
/abyss/          # Root directory
├── demons/      # System processes
├── souls/       # User data
├── grimoire/    # System configuration
├── artifacts/   # Applications
├── torment/     # Temporary files
└── purgatory/   # Recycle bin
```

## Color Palette

HellOS uses a 32-color palette with hellish themes:

- **Hell Red** (#FF0000) - Primary color
- **Flame Orange** (#FF4500) - Accent color
- **Void Black** (#000000) - Background
- **Sulfur Yellow** (#FFFF00) - Highlights
- **Bone White** (#FFFFFF) - Text
- And 27 more infernal colors...

## Audio System

3-channel audio with hellish sound effects:
- **Channel 0**: Square wave (harsh, demonic sounds)
- **Channel 1**: Sine wave (smooth, ethereal tones)
- **Channel 2**: Sawtooth wave (aggressive, cutting sounds)

## Development Status

### Completed ✅
- [x] Project structure and build system
- [x] UEFI bootloader foundation
- [x] Kernel core with hellish theming
- [x] Graphics driver (680x480, 32 colors)
- [x] Audio driver (3-channel synthesis)
- [x] Infernal Shell with demonic commands
- [x] Basic window management stubs

### In Progress 🔥
- [ ] Memory management system
- [ ] Interrupt handling
- [ ] Process management
- [ ] File system implementation
- [ ] Network stack (TCP/UDP)

### Planned 📋
- [ ] Complete window manager with flame effects
- [ ] Demonic applications suite
- [ ] Network utilities
- [ ] System configuration tools
- [ ] Performance optimizations

## Contributing

Join the infernal development! This OS is designed to be educational and fun while maintaining a hellish aesthetic throughout.

### Code Style
- Assembly for low-level operations
- C for high-level functionality
- Hellish naming conventions (demons, souls, realms)
- Extensive comments and documentation

## License

This project is released under the MIT License. May your code burn eternal! 🔥

## Acknowledgments

- Inspired by TempleOS by Terry A. Davis
- Built with passion for operating system development
- Themed for those who appreciate the darker side of computing

---

*"In the depths of silicon and fire, HellOS rises to torment the digital realm."*

**Warning**: This is an educational/experimental operating system. Use at your own risk in the digital underworld! 😈 