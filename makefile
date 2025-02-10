ERROR_FLAGS = -Wall -Werror -pedantic -std=c89
OPTIMIZATION_FLAGS = -Ofast

all: emulator assembler

emulator:
	gcc emulator.c -o emulator $(OPTIMIZATION_FLAGS) $(ERROR_FLAGS)

assembler:
	gcc assembler.c -o assembler $(OPTIMIZATION_FLAGS) $(ERROR_FLAGS)

clean:
	rm -f emulator assembler compiler *.bin

