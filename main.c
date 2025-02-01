#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<unistd.h>

typedef uint8_t byte;
struct timespec ts;
#define CLOCK_FREQUENCY 25
#define CLOCK_CYCLE() (usleep(1000000.0 / CLOCK_FREQUENCY))

#define MEMORY_SIZE 0xFF

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

void load_test_program(struct CPU* p_cpu)
{
	p_cpu->MEM[0x00] = INP;
	p_cpu->MEM[0x01] = STA;
	p_cpu->MEM[0x02] = 0x40;
	p_cpu->MEM[0x03] = INP;
	p_cpu->MEM[0x04] = ADD;
	p_cpu->MEM[0x05] = 0x40;
	p_cpu->MEM[0x06] = OUT;
	p_cpu->MEM[0x07] = BRZ;
	p_cpu->MEM[0x08] = 0x0E;
	p_cpu->MEM[0x09] = SUB;
	p_cpu->MEM[0x0A] = 0xF0;
	p_cpu->MEM[0x0B] = OUT;
	p_cpu->MEM[0x0C] = BRA; 
	p_cpu->MEM[0x0D] = 0x07;
	p_cpu->MEM[0x0E] = HLT;
	p_cpu->MEM[0xF0] = 1;
}

void reset_cpu(struct CPU* p_cpu)
{
	p_cpu->PC = 0x00;
	p_cpu->A = 0x00;
	p_cpu->B = 0x00;
	p_cpu->MAR = 0x00;
	p_cpu->MDR = 0x00;
	p_cpu->IR = 0x00;
	p_cpu->IOR = 0x00;

	for(int i = 0; i < MEMORY_SIZE; i++)
	{
		p_cpu->MEM[i] = 0x00;
	}
}

void MOV_PC_MAR(struct CPU* p_cpu)
{
	p_cpu->MAR = p_cpu->PC;
}

void INC_PC(struct CPU* p_cpu)
{
	p_cpu->PC += 1;
}

void READ(struct CPU* p_cpu)
{
	p_cpu->MDR = p_cpu->MEM[p_cpu->MAR];
	CLOCK_CYCLE();
}

void MOV_MDR_IR(struct CPU* p_cpu)
{
	p_cpu->IR = p_cpu->MDR;
}

void MOV_MDR_MAR(struct CPU* p_cpu)
{
	p_cpu->MAR = p_cpu->MDR;
}

void MOV_MDR_A(struct CPU* p_cpu)
{
	p_cpu->A = p_cpu->MDR;
}

void MOV_A_MDR(struct CPU* p_cpu)
{
	p_cpu->MDR = p_cpu->A;
}

void WRITE(struct CPU* p_cpu)
{
	p_cpu->MEM[p_cpu->MAR] = p_cpu->MDR;
	CLOCK_CYCLE();
}

void MOV_MDR_B(struct CPU* p_cpu)
{
	p_cpu->B = p_cpu->MDR;
}

void ALU_ADD(struct CPU* p_cpu)
{
	p_cpu->A += p_cpu->B;
}

void ALU_SUB(struct CPU* p_cpu)
{
	p_cpu->A -= p_cpu->B;
}

void WRITE_IOR(struct CPU* p_cpu, byte value)
{
	p_cpu->IOR = value;
}

void MOV_IOR_A(struct CPU* p_cpu)
{
	p_cpu->A = p_cpu->IOR;
}

void MOV_A_IOR(struct CPU* p_cpu)
{
	p_cpu->IOR = p_cpu->A;
}

void MOV_MAR_PC(struct CPU* p_cpu)
{
	p_cpu->PC = p_cpu->MAR;
}

void OUT_IOR(struct CPU* p_cpu)
{
	printf("OUT: %02X\n", p_cpu->IOR);	
}

int main(int argc, char* argv[])
{
	struct CPU cpu;
	reset_cpu(&cpu);
	load_test_program(&cpu);
	while(1)
	{
		//Fetch
		MOV_PC_MAR(&cpu);
		INC_PC(&cpu);
		READ(&cpu);
		MOV_MDR_IR(&cpu);

		//Decode & Execute
		switch(cpu.IR)
		{
			case HLT:
				goto end_execution;
			case LDA:
				MOV_PC_MAR(&cpu);
				INC_PC(&cpu);
				READ(&cpu);
				MOV_MDR_MAR(&cpu);
				READ(&cpu);
				MOV_MDR_A(&cpu);
				break;
			case STA:
				MOV_PC_MAR(&cpu);
				INC_PC(&cpu);
				READ(&cpu);
				MOV_MDR_MAR(&cpu);
				MOV_A_MDR(&cpu);
				WRITE(&cpu);
				break;
			case ADD:
				MOV_PC_MAR(&cpu);
				INC_PC(&cpu);
				READ(&cpu);
				MOV_MDR_MAR(&cpu);
				READ(&cpu);
				MOV_MDR_B(&cpu);
				ALU_ADD(&cpu);
				break;
			case SUB:
				MOV_PC_MAR(&cpu);
				INC_PC(&cpu);
				READ(&cpu);
				MOV_MDR_MAR(&cpu);
				READ(&cpu);
				MOV_MDR_B(&cpu);
				ALU_SUB(&cpu);
				break;
			case INP:
				char input[2];
					
				printf("INP: ");
				scanf("%s", input);
				printf("\n");
				
				byte value = (byte)strtol(input, NULL, 16);
				WRITE_IOR(&cpu, value);
				MOV_IOR_A(&cpu);
				break;
			case OUT:
				MOV_A_IOR(&cpu);
				OUT_IOR(&cpu);
				break;
			case BRZ:
				if(cpu.A == 0) {
					MOV_PC_MAR(&cpu);
					READ(&cpu);
					MOV_MDR_MAR(&cpu);
					MOV_MAR_PC(&cpu); }
				break;
			case BRP:
				if(cpu.A >= 0) {
					MOV_PC_MAR(&cpu);
					READ(&cpu);
					MOV_MDR_MAR(&cpu);
					MOV_MAR_PC(&cpu); }
				break;
			case BRA:
				MOV_PC_MAR(&cpu);
				READ(&cpu);
				MOV_MDR_MAR(&cpu);
				MOV_MAR_PC(&cpu);
				break;
		}
	}
	end_execution:
	return 0;
}
