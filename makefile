all: main assembler

debug_all: debug_main debug_assembler

main:
	gcc main.c -o main

assembler:
	gcc assembler.c -o assembler

debug_main:
	gcc main.c -o main -ggdb

debug_assembler:
	gcc assembler.c -o assembler -ggdb

clean:
	rm -f main assembler rom.bin
