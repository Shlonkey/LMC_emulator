#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<unistd.h>
#include<stdint.h>
#include<string.h>

#include"opcodes.h"

struct Variable
{
	char* name;
	unsigned char length;

	byte value;
	byte address;
};

struct Lable
{
	char* name;
	unsigned char length;

	byte address;
};

struct Token
{
	char* name;
	unsigned char length;
};

struct Token_Counts
{
	unsigned char number_of_variables;
	unsigned char number_of_lables;
	unsigned char number_of_instruction_tokens;
};

byte decode_opcode(char* token)
{
	if(strcmp(token, "HLT") == 0)
		return HLT;
	if(strcmp(token, "LDA") == 0)
		return LDA;
	if(strcmp(token, "STA") == 0)
		return STA;
	if(strcmp(token, "ADD") == 0)
		return ADD;
	if(strcmp(token, "SUB") == 0)
		return SUB;
	if(strcmp(token, "INP") == 0)
		return INP;
	if(strcmp(token, "OUT") == 0)
		return OUT;
	if(strcmp(token, "BRZ") == 0)
		return BRZ;
	if(strcmp(token, "BRP") == 0)
		return BRP;
	if(strcmp(token, "BRA") == 0)
		return BRA;
	else
		return NOT_AN_INSTRUCTION;
}

void replace(char* source, char char_to_replace, char char_to_replace_with)
{
	unsigned char i = 0;
	while(source[i] != '\0')
	{
		source[i] = (source[i] == char_to_replace) ? char_to_replace_with : source[i];
		i++;
	}
	return;
}

char* unpair_lables(char* source)
{
	unsigned char number_of_lables = 0;
	unsigned char number_of_chars = 0;
	unsigned char index;
	char* new_char_stream;
	char c;
	unsigned char offset;

	for(index = 0; source[index] != '\0'; index++, number_of_chars++)
	{
		number_of_lables += (source[index] == ':' ? 1 : 0);
	}
	
	new_char_stream = (char*)malloc(sizeof(char) * (number_of_chars + number_of_lables));
	
	for(index = 0, offset = 0; index < number_of_chars; index++)
	{
		c = source[index];
		
		new_char_stream[index + offset] = c;
		if(c == ':') {
			new_char_stream[index + offset + 1] = ' ';
			offset++; }
	}
	return new_char_stream;
}

char* remove_comments(char* char_stream, unsigned int char_stream_length)
{
	unsigned int new_char_stream_length = 0;
	bool in_comment = false;
	unsigned int i;
	char c;
	char* new_char_stream;
	unsigned int new_i;

	for(i = 0; i < char_stream_length; i++)
	{
		c = char_stream[i];
		if(c == ';')
			in_comment = true;
		else if(c == '\n')
			in_comment = false;
		if(in_comment == false)
			   new_char_stream_length++;	
	}
	new_char_stream = (char*)malloc(sizeof(char) * new_char_stream_length);
	in_comment = false;
	for(i = 0, new_i = 0; i < char_stream_length; i++)
	{
		c = char_stream[i];
		
		if(c == ';')
			in_comment = true;
		else if(c == '\n')
			in_comment = false;
		if(!in_comment) {
			new_char_stream[new_i] = c;
			new_i++; }
	}

	return new_char_stream;
}

char* remove_repeated_whitespace(char* char_stream)
{
	
	unsigned int char_count = 0;
	unsigned int index = 0;
	unsigned int new_index = 0;
	char* new_char_stream;	

	if (!char_stream) return NULL;
	
	while (char_stream[index] != '\0') {
		if (!(char_stream[index] == ' ' && char_stream[index + 1] == ' ')) {
			char_count++;
		}
		index++;
	}

	new_char_stream = (char*)malloc(sizeof(char) * (char_count + 1));
	if (!new_char_stream) return NULL; /* Handle memory allocation failure*/

	index = 0;

	while (char_stream[index] != '\0') {
		if (!(char_stream[index] == ' ' && char_stream[index + 1] == ' ')) {
			new_char_stream[new_index++] = char_stream[index];
		}
		index++;
	}

	new_char_stream[new_index] = '\0';

	return new_char_stream;
}

char* strip(char* non_stripped_stream)
{
	unsigned int length = 0;
	unsigned int i;
	char c;

	unsigned int first_ws;
	unsigned int last_ws;
	unsigned int chars_to_remove;
	unsigned int new_length;
	char* stripped_stream;
	unsigned int offset;

	for(i = 0; non_stripped_stream[i] != '\0'; i++)
	{
		length++;
	}

	first_ws = (non_stripped_stream[0] == ' ' ? 1 : 0);
	last_ws = (non_stripped_stream[length - 1] == ' ' ? 1 : 0);
	chars_to_remove = first_ws + last_ws;
	new_length = length - chars_to_remove;
	stripped_stream = (char*)malloc(sizeof(char) * new_length);
	offset = 0;

	for(i = 0; non_stripped_stream[i] != '\0'; i++)
	{
		c = non_stripped_stream[i];
		if(i == 0 && c == ' ') {
			offset++;
			continue; }
		else if(i == length - 1 && c == ' ') {
			offset++;
			continue; }
		stripped_stream[i - offset] = c;
	}
	return stripped_stream;
}

size_t split_stream_by_char(char* stream, struct Token** p_tokens, char c)
{
	unsigned char token_count = 1;	
	unsigned int stream_index;
	unsigned char token_index;
	char current_char;
	unsigned char token_offset;
	struct Token* local_out_tokens;
	struct Token* p_token;
	unsigned char within_token_char_index = 0;

	for(stream_index = 0; stream[stream_index] != '\0'; stream_index++)
	{
		token_count += (stream[stream_index] == c ? 1 : 0);
	}

	local_out_tokens = (struct Token*)malloc(sizeof(struct Token) * token_count);

	for(stream_index = 0, token_index = 0; stream[stream_index] != '\0'; stream_index++)
	{
		current_char = stream[stream_index];
		if(current_char == c) {
			token_index++;
		} else {
			local_out_tokens[token_index].length++;
		}
	}
	
	for(token_index = 0, token_offset = 0; token_index < token_count; token_index++)
	{
		local_out_tokens[token_index].name = (char*)malloc(sizeof(char) * local_out_tokens[token_index].length);
	}
	
	for(token_index = 0; token_index < token_count; token_index++)
	{
		p_token = local_out_tokens + token_index;
		for(within_token_char_index = 0; within_token_char_index < p_token->length; within_token_char_index++)
		{
			p_token->name[within_token_char_index] = stream[token_offset + within_token_char_index];
		}
		token_offset += (p_token->length + 1);
	}

	*p_tokens = local_out_tokens;
	return token_count;
}

void calculate_token_type_counts(struct Token* tokens, unsigned char num_tokens, struct Token_Counts* p_token_counts)
{
	unsigned char number_of_variables = 0;
	unsigned char number_of_lables = 0;
	unsigned char number_of_tokens_corresponding_to_variables = 0;
	unsigned char token_index;
	struct Token token;

	for(token_index = 0; token_index < num_tokens; token_index++)
	{
		token = tokens[token_index];
		if(token.name[token.length - 1] == ':')
		{
			number_of_lables++;
			continue; }
		else if(token.length != 3) {
			continue; }
		else if(strcmp(token.name, "DAT") == 0){
			number_of_variables++;
			number_of_tokens_corresponding_to_variables += 2;
			token_index++;
			if(token_index < num_tokens - 1)
			{
				if(tokens[token_index + 1].name[0] == '0' && tokens[token_index + 1].name[1] == 'x')
				{
					number_of_tokens_corresponding_to_variables++;
					token_index++;
				}
			}
		}

	}
	
	p_token_counts->number_of_variables = number_of_variables;
	p_token_counts->number_of_lables = number_of_lables;
	p_token_counts->number_of_instruction_tokens = num_tokens - number_of_tokens_corresponding_to_variables - number_of_lables;
	
	return;
}

void split_tokens_by_type(struct Token* tokens, unsigned char num_tokens, struct Variable* variables, struct Lable* lables, struct Token* instruction_tokens)
{
	byte location_counter = 0;
	unsigned char variable_index = 0;
	unsigned char lable_index = 0;
	unsigned char instruction_index = 0;
	unsigned char token_index;
	unsigned char token_length;
	char* variable_name;
	char* default_value;

	struct Token token;
	struct Lable* p_lable;
	struct Variable* p_variable;
	struct Token* p_instruction_token; 


	for(token_index = 0; token_index < num_tokens; token_index++)
	{
		token = tokens[token_index];
		token_length = token.length;
		if(token.name[token_length - 1] == ':') {/*Lable line*/
			p_lable = lables + lable_index;
			p_lable->length = token_length - 1;
			p_lable->name = (char*)malloc(sizeof(char) * token_length - 1);
		
			memcpy(p_lable->name, token.name, token_length - 1);
			
			p_lable->address = location_counter;
			lable_index++;
			continue; 
		} if(token_length == 3 && (strcmp(token.name, "DAT") == 0)) {
			token_index++;
			token = tokens[token_index];
			p_variable = variables + variable_index;
			variable_name = token.name;
			token_length = token.length;
			p_variable->length = token_length;
			p_variable->name = (char*)malloc(sizeof(char) * token_length);
			
			memcpy(p_variable->name, variable_name, token_length);
			
			if(token_index < num_tokens - 1)
			{
				default_value = tokens[token_index + 1].name;
				if(default_value[0] == '0' && default_value[1] == 'x')
				{
					byte value = strtol(default_value, NULL, 16);
					p_variable->value = value;
					token_index++;
				} else {
					p_variable->value = 0xEE;
				}
			}
			variable_index++;
			continue;
		} else {
			p_instruction_token = instruction_tokens + instruction_index;
			p_instruction_token->length = token_length;
			p_instruction_token->name = (char*)malloc(sizeof(char) * token_length);
			
			memcpy(p_instruction_token->name, token.name, token_length);
			
			instruction_index++;
			location_counter++; 
		}
	}	
}

void set_variable_addresses(struct Variable* variables, unsigned char number_of_variables)/*Could put variables directly after instructions, however this is nice for now.*/
{
	unsigned char variable_index;
	for(variable_index = 0; variable_index < number_of_variables; variable_index++)
	{
		variables[variable_index].address = MEMORY_SIZE - variable_index - 1;	
	}
}

void replace_lables_with_address(byte* program, struct Lable* lables, unsigned char number_of_lables, struct Token* instruction_tokens, unsigned char number_of_instruction_tokens)
{
	unsigned char token_index;
	unsigned char lable_index;

	struct Token token;
	struct Lable lable;

	for(token_index = 0; token_index < number_of_instruction_tokens; token_index++)
	{
		token = instruction_tokens[token_index];
		for(lable_index = 0; lable_index < number_of_lables; lable_index++)
		{
			lable = lables[lable_index];
			if(strcmp(lable.name, token.name) == 0) {
				program[token_index] = lable.address;
				break;
			}
		}
	}
}

void replace_variables_with_address(byte* program, struct Variable* variables, unsigned char number_of_variables, struct Token* instruction_tokens, unsigned char number_of_instruction_tokens)
{
	unsigned char token_index;
	unsigned char variable_index;
	struct Token token;
	struct Variable variable;

	for(token_index = 0; token_index < number_of_instruction_tokens; token_index++)
	{
		token = instruction_tokens[token_index];
		for(variable_index = 0; variable_index < number_of_variables; variable_index++)
		{
			variable = variables[variable_index];
			if(strcmp(variable.name, token.name) == 0) {
				program[token_index] = variable.address;
				break;
			}
		}
	}
}

void replace_instruction_with_opcode(byte* program, struct Token* instruction_tokens, unsigned char number_of_instruction_tokens)
{
	unsigned char token_index;
	struct Token instruction_token;
	byte opcode;

	for(token_index = 0; token_index < number_of_instruction_tokens; token_index++)
	{
		instruction_token = instruction_tokens[token_index];
		opcode = decode_opcode(instruction_token.name);
		/*if token in pneumonics, convert.*/
		if(opcode != NOT_AN_INSTRUCTION) {
			program[token_index] = opcode;
		} else if(instruction_token.name[0] == '0' && instruction_token.name[1] == 'x') {
			program[token_index] = strtol(instruction_token.name, NULL, 16); }
	}
}

void translate_to_machine_code(byte* program, struct Token* instruction_tokens, struct Lable* lables, struct Variable* variables, struct Token_Counts token_counts)
{
	replace_lables_with_address(program, lables, token_counts.number_of_lables, instruction_tokens, token_counts.number_of_instruction_tokens);
	replace_variables_with_address(program, variables, token_counts.number_of_variables, instruction_tokens, token_counts.number_of_instruction_tokens);
	replace_instruction_with_opcode(program, instruction_tokens, token_counts.number_of_instruction_tokens);
}

void load_initial_variable_values(byte* program, struct Variable* variables, unsigned char number_of_variables)
{
	unsigned char variable_index;
	struct Variable variable;

	for(variable_index = 0; variable_index < number_of_variables; variable_index++)
	{
		variable = variables[variable_index];
		program[variable.address] = variable.value;
	}
}

void free_variables(struct Variable* variables, unsigned char count)
{
	unsigned char index;
	for(index = 0; index < count; index++)
	{
		free(variables[index].name);	
	}
	free(variables);
}

void free_lables(struct Lable* lables, unsigned char count)
{
	unsigned char index;
	for(index = 0; index < count; index++)
	{
		free(lables[index].name);
	}
	free(lables);
}

void free_tokens(struct Token* tokens, unsigned char count)
{
	unsigned char index;
	for(index = 0; index < count; index++)
	{
		free(tokens[index].name);
	}
	free(tokens);
}

int main(int argc, char* argv[])
{
	struct stat sb;
	int fd;
	int file_size;
	char* address;
	char* char_stream_without_comments;
	char* char_stream_with_comments;
	char* new_char_stream;
	char* token_char_stream_not_stripped;
	char* token_char_stream_stripped;
	struct Token* tokens;
	unsigned char num_tokens;
	struct Token_Counts token_counts;
	struct Variable* variables;
	struct Lable* lables;
	struct Token* instruction_tokens;
	byte* program;
	FILE* p_file;

	fd = open(argv[1], O_RDONLY);
		fstat(fd, &sb);		   /* To obtain file size */
	file_size = sb.st_size;
	address = (char*)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
	
	char_stream_with_comments = (char*)malloc(sizeof(char) * file_size);
	memcpy(char_stream_with_comments, address, file_size);
	
	munmap(address, file_size);
	close(fd);

	char_stream_without_comments = remove_comments(char_stream_with_comments, file_size);
	free(char_stream_with_comments);
	replace(char_stream_without_comments, '\t', ' ');
	replace(char_stream_without_comments, '\n', ' ');
	
	new_char_stream = unpair_lables(char_stream_without_comments);
	free(char_stream_without_comments);
	
	token_char_stream_not_stripped = remove_repeated_whitespace(new_char_stream);
	free(new_char_stream);
	

	token_char_stream_stripped = strip(token_char_stream_not_stripped);
	free(token_char_stream_not_stripped);

	num_tokens = split_stream_by_char(token_char_stream_stripped, &tokens, ' ');
	
	free(token_char_stream_stripped);

	calculate_token_type_counts(tokens, num_tokens, &token_counts);

	variables = (struct Variable*)malloc(sizeof(struct Variable) * token_counts.number_of_variables);
	lables = (struct Lable*)malloc(sizeof(struct Lable) * token_counts.number_of_lables);
	instruction_tokens = (struct Token*)malloc(sizeof(struct Token) * token_counts.number_of_instruction_tokens);

	split_tokens_by_type(tokens, num_tokens, variables, lables, instruction_tokens);
	
	free(tokens);
	
	set_variable_addresses(variables, token_counts.number_of_variables);	

	program = (byte*)malloc(sizeof(byte) * MEMORY_SIZE);
	translate_to_machine_code(program, instruction_tokens, lables, variables, token_counts);
	
	load_initial_variable_values(program, variables, token_counts.number_of_variables);
	
	free_variables(variables, token_counts.number_of_variables);
	free_lables(lables, token_counts.number_of_lables);
	free_tokens(instruction_tokens, token_counts.number_of_instruction_tokens);

	p_file = fopen(argv[2], "wb");
	fwrite(program, sizeof(byte), MEMORY_SIZE, p_file);
	fclose(p_file);

	free(program);
	
	return 0;
}
