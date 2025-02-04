#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<unistd.h>

#include"opcodes.h"

#define CLOCK_FREQUENCY 25
#define CLOCK_CYCLE() (usleep(1000000.0 / CLOCK_FREQUENCY))

void load_rom(struct CPU* p_cpu, char* path)
{
	FILE* p_file = fopen(path, "rb");
	byte* program = (byte*)malloc(sizeof(byte) * MEMORY_SIZE);
	fread(program, sizeof(byte), MEMORY_SIZE, p_file);
       	fclose(p_file);	
	
	for(size_t i = 0; i < MEMORY_SIZE; i++)
	{
		p_cpu->MEM[i] = program[i];
	}
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
	printf("OUT: %i\n", p_cpu->IOR);	
}

int main(int argc, char* argv[])
{
	struct CPU cpu;
	reset_cpu(&cpu);
	load_rom(&cpu, argv[1]);

	size_t total_instructions_run = 0;

	while(1)
	{
		//Fetch
		MOV_PC_MAR(&cpu);
		INC_PC(&cpu);
		READ(&cpu);
		MOV_MDR_IR(&cpu);
		//Decode & Execute
		total_instructions_run++;
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
	printf("\n\nProgram Terminated in %i Cycles\n\n", total_instructions_run);
	return 0;
}
