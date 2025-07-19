# HellOS - Recreational  Operating System

## Dependencies
- nasm
- qemu
- gcc
- gdb (optonal for debugging stuff)

## Commands

### - Build os
```
nasm -f bin -o os.asm -o os.bin
```

### - Run QEMU
```
qemu-system-x86_64 -fda os.bin
```