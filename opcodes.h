#ifndef HARDWARE_H
#define HARDWARE_H

#define HLT 0x0
#define LDA 0x1
#define STA 0x2
#define ADD 0x3
#define SUB 0x4
#define INP 0x5
#define OUT 0x6
#define BRZ 0x7
#define BRP 0x8
#define BRA 0x9
#define NOT_AN_INSTRUCTION 0xFF

#define MEMORY_SIZE 0x100

typedef uint8_t byte;

struct CPU {
	byte PC;	//Program Counter
	byte A;		//Accumulator
	byte B;		//Secondary ALU register
	byte MAR;	//Memory Address Register
	byte MDR;	//Memory Data Register
	byte IR;	//Instruction Register
	byte IOR;	//User IO Register
	byte MEM[MEMORY_SIZE];	//Main Memory
};

#endif
