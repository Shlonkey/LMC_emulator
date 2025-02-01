all: main assembler

debug_all: debug_main debug_assembler

main:
	gcc main.c -o main.x86_64

assembler:
	gcc assembler.c -o assembler.x86_64

debug_main:
	gcc main.c -o main.x86_64 -ggdb

debug_assembler:
	gcc assembler.c -o assembler.x86_64 -ggdb

clean:
	rm -f main.x86_64 assembler.x86_64 rom.bin
