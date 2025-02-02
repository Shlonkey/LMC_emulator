all: enumator assembler compiler

emulator:
	gcc emulator.c -o emulator

assembler:
	gcc assembler.c -o assembler

compiler:
	gcc compiler.c -o compiler

clean:
	rm -f emulator assembler compiler

