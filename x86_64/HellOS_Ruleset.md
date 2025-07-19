# HellOS Ruleset

## Design Philosophy
HellOS is a modern x86_64 operating system inspired by TempleOS with a hellish aesthetic, emphasizing simplicity, performance, and a unique user experience through infernal theming and proprietary technologies.

## Core Specifications

### Architecture
- **Target**: x86_64 (64-bit) - Currently targeting x86 (32-bit) for initial implementation
- **Boot Method**: El Torito CD-ROM boot (UEFI planned for later phases)
- **Kernel Type**: Monolithic with modular drivers
- **Memory Model**: Flat memory model with paging

### Graphics System
- **Resolution**: 680x480 pixels
- **Color Depth**: 32 colors (5-bit palette)
- **Rendering**: Custom proprietary graphics engine
- **Theme**: Hellish aesthetics (reds, blacks, oranges, fire effects)

### Audio System
- **Channels**: 3 simultaneous audio channels
- **Waveforms**: Square, Sine, Sawtooth
- **Sample Rate**: 44.1 kHz
- **Bit Depth**: 16-bit

### Network Stack
- **Protocols**: TCP, UDP
- **Sockets**: BSD-style socket API
- **Ethernet**: Basic Ethernet driver support
- **IP**: IPv4 support

## System Architecture

### Boot Process
1. **Unified Bootloader**: Single bootloader supporting CD-ROM boot with El Torito
2. **Kernel Init**: Memory management, interrupt handlers, debug system initialization
3. **Driver Loading**: Graphics, audio, network drivers with debug integration
4. **Shell Launch**: Multi-window infernal shell

### Memory Management
- **Paging**: 4KB pages with 4-level page tables
- **Heap**: Custom heap allocator
- **Stack**: Per-process stacks with guard pages
- **Virtual Memory**: Full virtual memory support

### Process Management
- **Scheduling**: Round-robin with priority levels
- **IPC**: Message passing between processes
- **Threads**: Cooperative threading model

### Debug System
HellOS includes a comprehensive global debugging and logging system:

#### Debug Levels
- **TRACE**: Detailed execution flow
- **DEBUG**: Development information
- **INFO**: General information
- **WARN**: Warning messages
- **ERROR**: Error conditions
- **FATAL**: Critical system failures

#### Debug Subsystems
- **BOOT**: Bootloader debugging
- **KERNEL**: Kernel core debugging
- **MEMORY**: Memory management debugging
- **PROCESS**: Process management debugging
- **INTERRUPT**: Interrupt handling debugging
- **DRIVER**: Device driver debugging
- **GRAPHICS**: Graphics system debugging
- **AUDIO**: Audio system debugging
- **NETWORK**: Network stack debugging
- **SHELL**: Shell debugging
- **FILESYSTEM**: File system debugging

#### Debug Output Targets
- **VGA**: Text mode output to screen
- **SERIAL**: Serial port output (COM1)
- **MEMORY**: In-memory log buffer

#### Debug Configuration
- Compile-time configuration for debug levels
- Runtime configuration for output targets
- Per-subsystem debug control
- Memory dump capabilities
- Panic handling with detailed error reporting

## Current Codebase Structure

### Boot Directory (`/boot/`)
- `unified_bootloader.asm`: Single bootloader supporting El Torito CD-ROM boot
  - Integrated debug system with serial output
  - Optimized for 512-byte boot sector constraint
  - Loads kernel and transfers control

### Kernel Directory (`/kernel/`)
#### Core Kernel Files
- `kernel.c`: Main kernel initialization and core functions
- `kernel.h`: Kernel interface definitions
- `kernel_entry.asm`: Assembly entry point for kernel
- `kernel.ld`: Linker script for kernel layout

#### Memory Management
- `memory.c`: Memory management implementation
- `memory.h`: Memory management interface
- `memory_layout.h`: Memory layout definitions

#### Process Management
- `process.c`: Process management implementation
- `process.h`: Process management interface

#### Interrupt Handling
- `interrupts.c`: Interrupt handling implementation
- `interrupts.h`: Interrupt handling interface

#### Debug System
- `debug.c`: Global debug system implementation (14KB, 490 lines)
- `debug.h`: Debug system interface and macros (6.1KB, 175 lines)
- Comprehensive logging, VGA/serial output, memory dumps
- Boot-time debugging functions
- Panic handling with detailed error reporting

#### Stub Implementations
- `stubs.c`: Temporary implementations for missing functions
- Window manager stubs
- Process management stubs
- Driver management stubs

#### Hardware Interface
- `audio.h`: Audio system interface
- `graphics.h`: Graphics system interface

### Tools Directory (`/tools/`)
Development and debugging utilities:
- `debug_viewer.c`: Log analysis tool with filtering and statistics
- `memory_analyzer.c`: Memory dump analysis with hex display
- `boot_checker.c`: Boot sector validation and analysis
- `Makefile`: Build system for development tools

### Drivers Directory (`/drivers/`)
Modular driver system with debug integration:
- `audio/hell_audio.c`: Audio driver implementation
- `graphics/hell_graphics.c`: Graphics driver implementation
- `network/hell_network.c`: Network driver implementation

### Shell Directory (`/shell/`)
- `infernal_shell.c`: Command-line interface implementation

### Build System
- `Makefile`: Comprehensive build system with debug support
- Clean build targets for production and debug builds
- Automatic dependency management
- ISO image generation with proper El Torito boot
- Tools compilation support

## Hellish Theme Guidelines

### Visual Design
- **Primary Colors**: #FF0000 (Hell Red), #000000 (Void Black), #FF4500 (Flame Orange)
- **Secondary Colors**: #8B0000 (Dark Red), #2F0000 (Deep Crimson)
- **Fonts**: Gothic/Medieval style fonts
- **Icons**: Demonic symbols, flames, skulls, pentagrams

### Audio Design
- **System Sounds**: Demonic growls, fire crackling, metal clanging
- **Startup Sound**: Ominous organ chord progression
- **Error Sounds**: Screams, thunder, demonic laughter

### Naming Conventions
- **Processes**: Demons, Devils, Infernal beings
- **Files**: Hellish terminology (brimstone, sulfur, abyss)
- **Commands**: Latin/demonic names (infernus, daemon, maleficus)

## File System

### Structure
```
/abyss/          # Root directory
├── demons/      # System processes
├── souls/       # User data
├── grimoire/    # System configuration
├── artifacts/   # Applications
├── torment/     # Temporary files
└── purgatory/   # Recycle bin
```

### File Types
- `.soul` - Executable files
- `.hex` - Configuration files
- `.rune` - Script files
- `.tome` - Document files

## Command Line Interface

### Shell: "Infernal Shell" (InfSh)
- **Prompt**: `🔥 [current_realm] λ `
- **Commands**: Hellish-themed equivalents of standard commands
  - `summon` (run/execute)
  - `banish` (kill/terminate)
  - `scry` (list/view)
  - `conjure` (create)
  - `purge` (delete)

## Window Manager: "Pandemonium WM"

### Features
- **Multi-window**: Overlapping windows with hellish borders
- **Transparency**: Flame-like transparency effects
- **Animations**: Fire and smoke transition effects
- **Themes**: Multiple hellish themes (Inferno, Abyss, Purgatory)

### Window Decorations
- **Borders**: Flame-like borders with ember particles
- **Title Bars**: Gothic style with demonic symbols
- **Buttons**: Skull and crossbones for close, minimize, maximize

## Development Guidelines

### Code Style
- **Language**: Assembly for low-level, C for high-level
- **Naming**: snake_case for C, descriptive names
- **Comments**: Extensive documentation
- **Error Handling**: Comprehensive error codes with hellish names
- **Debug Integration**: All code includes debug logging

### Build System
- **Compiler**: GCC cross-compiler for x86 (32-bit currently)
- **Assembler**: NASM for assembly code
- **Linker**: Custom linker script
- **Build Tool**: Make with custom Makefile
- **Debug Builds**: Separate debug targets with symbols
- **Tools**: Integrated development tool compilation

### Debug Development
- **Global Debug System**: Comprehensive logging throughout codebase
- **Debug Levels**: Configurable debug output levels
- **Multiple Targets**: VGA, serial, and memory output
- **Development Tools**: Specialized debugging utilities
- **Memory Analysis**: Built-in memory dump and analysis
- **Boot Debugging**: Serial output during boot process

## Security Model

### Permissions
- **Souls**: User-level permissions
- **Demons**: System-level permissions
- **Overlords**: Administrator permissions
- **The Dark Lord**: Root/kernel permissions

### Access Control
- **Realms**: Isolated execution environments
- **Wards**: Protection mechanisms
- **Seals**: Cryptographic security

## Performance Targets

### Boot Time
- **Cold Boot**: < 10 seconds
- **Warm Boot**: < 5 seconds

### Memory Usage
- **Kernel**: < 16MB base footprint
- **Graphics**: < 8MB VRAM usage
- **Audio**: < 2MB buffer space

### Responsiveness
- **UI Response**: < 50ms for basic operations
- **Audio Latency**: < 10ms
- **Network**: Standard TCP/IP performance

## Implementation Phases

### Phase 1: Foundation ✅ COMPLETED
- ✅ Unified bootloader with El Torito CD-ROM boot
- ✅ Kernel base with entry point and initialization
- ✅ Basic memory management
- ✅ Interrupt handling system
- ✅ Global debug system with comprehensive logging
- ✅ Build system with debug support
- ✅ Development tools for debugging and analysis
- ✅ Code cleanup and redundancy removal

### Phase 2: Drivers (IN PROGRESS)
- 🔄 Graphics driver implementation (basic structure exists)
- 🔄 Audio driver implementation (basic structure exists)
- 🔄 Network driver implementation (basic structure exists)
- ⏳ Basic input/output drivers
- ⏳ Hardware abstraction layer completion

### Phase 3: System Services (PLANNED)
- ⏳ Process management completion
- ⏳ File system implementation
- ⏳ Network stack completion
- ⏳ Inter-process communication

### Phase 4: User Interface (PLANNED)
- ⏳ Window manager implementation
- ⏳ Shell completion with hellish theming
- ⏳ Basic applications
- ⏳ Graphics rendering system

### Phase 5: Polish (PLANNED)
- ⏳ Theming and effects
- ⏳ Performance optimization
- ⏳ Documentation completion
- ⏳ UEFI boot support migration

## Quality Assurance

### Testing Strategy
- **Unit Tests**: For individual components
- **Integration Tests**: For system interactions
- **Performance Tests**: For meeting targets
- **Compatibility Tests**: For hardware support
- **Debug Testing**: Comprehensive debug system validation

### Documentation
- **Code Documentation**: Inline comments and headers
- **User Manual**: "The Infernal Grimoire"
- **Developer Guide**: "Summoning Demons: A Developer's Guide"
- **Debug Guide**: "Debugging the Depths: HellOS Debug System"

### Current Status
- **Build Status**: ✅ Clean compilation and linking
- **Boot Status**: ✅ Successfully boots and displays debug messages
- **Debug Status**: ✅ Comprehensive debug system integrated
- **Code Quality**: ✅ Redundant code removed, clean codebase
- **Development Tools**: ✅ Debug utilities available

---
*"In the depths of silicon and fire, HellOS rises to torment the digital realm."*

## Recent Changes (Latest Update)

### Codebase Cleanup
- Removed redundant bootloader files (8 files)
- Removed test and debug kernel files (4 files)
- Streamlined to single production bootloader and kernel
- Updated build system to remove obsolete targets

### Debug System Integration
- Implemented comprehensive global debug system
- Added debug levels, subsystems, and output targets
- Integrated debug logging throughout kernel and drivers
- Added boot-time debugging with serial output
- Created development tools for log analysis and debugging

### Build System Improvements
- Cleaned Makefile with proper dependency management
- Added debug build targets with symbols
- Integrated tools compilation
- Optimized ISO generation with proper El Torito boot
- Fixed all compilation and linking issues

### Testing and Validation
- ✅ System builds cleanly without errors
- ✅ System boots successfully with debug output
- ✅ Debug system functions correctly
- ✅ Development tools compile and function
- ✅ All redundant code successfully removed 