.PHONY: clean, .force-rebuild
all: bootloader.bin
		
bootloader.bin: os.asm .force-rebuild
	nasm -g -f elf64 -l 2.lst -fbin os.asm -o os.bin

clean:
	rm *.bin

