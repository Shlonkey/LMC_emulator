# LMC EMULATOR and ASSEMBLER
This repo contains 2 peices of software;  
An Emulator for the fictional 'Little Man Computer' (LMC) microprocessor & an Assembler for taking a file containing LMC instructions, and converting it into a binary file which can be loaded into the emulator 

# Setup
Setup is facilitated by a make file.
```bash 
make clean all
```

# Usage
Usage currently consists of 2 stages, using the assembler executable to read a file containing LMC instructions, and write it to a file. Then using the file as an input to the LMC emulator.

## Assembly
Assuming you have a file A.asm (extensions are not relevant)
```bash
./assembler A.asm A.bin
```
## Running
```
./emulator A.bin
```

# Example
Using one of the example assembly files, lets run all the steps assuming a completely clean install.

