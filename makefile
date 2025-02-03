all: emulator assembler

emulator:
	gcc emulator.c -o emulator

assembler:
	gcc assembler.c -o assembler

clean:
	rm -f emulator assembler compiler

