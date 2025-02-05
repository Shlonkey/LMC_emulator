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
	size_t length;

	byte value;
	byte address;
};

struct Lable
{
	char* name;
	size_t length;

	byte address;
};

struct Token
{
	char* name;
	size_t length;
};

struct Token_Counts
{
	size_t number_of_variables;
	size_t number_of_lables;
	size_t number_of_instruction_tokens;
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

void copy_char_array(char* source, char* dest, size_t length)
{
	for(size_t i = 0; i < length; i++)
	{
		dest[i] = source[i];
	}
	return;
}

void replace(char* source, char char_to_replace, char char_to_replace_with)
{
	size_t i = 0;
	while(source[i] != '\0')
	{
		source[i] = (source[i] == char_to_replace) ? char_to_replace_with : source[i];
		i++;
	}
	return;
}

char* unpair_lables(char* source)
{
	size_t number_of_lables = 0;
	size_t number_of_chars = 0;
	for(size_t index = 0; source[index] != '\0'; index++, number_of_chars++)
	{
		number_of_lables += (source[index] == ':' ? 1 : 0);
	}
	char* new_char_stream = (char*)malloc(sizeof(char) * (number_of_chars + number_of_lables));
	
	for(size_t index = 0, offset = 0; index < number_of_chars; index++)
	{
		char c = source[index];
		new_char_stream[index + offset] = c;
		if(c == ':') {
			new_char_stream[index + offset + 1] = ' ';
			offset++; }
	}
	return new_char_stream;
}

char* remove_comments(char* char_stream, size_t char_stream_length)
{
	size_t new_char_stream_length = 0;
	bool in_comment = false;
	for(size_t i = 0; i < char_stream_length; i++)
	{
		char c = char_stream[i];
		if(c == ';')
			in_comment = true;
		else if(c == '\n')
			in_comment = false;
		if(in_comment == false)
		       new_char_stream_length++;	
	}
	char* new_char_stream = (char*)malloc(sizeof(char) * new_char_stream_length);
	in_comment = false;
	size_t new_i = 0;
	for(size_t i = 0; i < char_stream_length; i++)
	{
		char c = char_stream[i];
		
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
    if (!char_stream) return NULL;

    size_t char_count = 0;
    size_t index = 0;

    while (char_stream[index] != '\0') {
        if (!(char_stream[index] == ' ' && char_stream[index + 1] == ' ')) {
            char_count++;
        }
        index++;
    }

    char* new_char_stream = (char*)malloc(sizeof(char) * (char_count + 1));
    if (!new_char_stream) return NULL; // Handle memory allocation failure

    index = 0;
    size_t new_index = 0;

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
	size_t length = 0;
	for(size_t i = 0; non_stripped_stream[i] != '\0'; i++)
	{
		length++;
	}

	int first_ws = (non_stripped_stream[0] == ' ' ? 1 : 0);
	int last_ws = (non_stripped_stream[length - 1] == ' ' ? 1 : 0);
	size_t chars_to_remove = first_ws + last_ws;
	size_t new_length = length - chars_to_remove;
	char* stripped_stream = (char*)malloc(sizeof(char) * new_length);
	int offset = 0;
	for(size_t i = 0; non_stripped_stream[i] != '\0'; i++)
	{
		char c = non_stripped_stream[i];
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

size_t split_stream_by_char(char* stream, char*** out_tokens, size_t** out_token_lengths, char c)
{
	size_t token_count = 1;	
	for(size_t index = 0; stream[index] != '\0'; index++)
	{
		token_count += (stream[index] == ' ' ? 1 : 0);
	}

	char** local_out_tokens = (char**)malloc(sizeof(char*) * token_count);
	size_t* split_lengths = malloc(sizeof(size_t) * token_count);

	for(size_t index = 0, token_index = 0; stream[index] != '\0'; index++)
	{
		char current_char = stream[index];
		if(current_char == ' ') {
			token_index++;
		} else {
			split_lengths[token_index]++;
		}
	}
	
	*out_token_lengths = split_lengths;

	for(size_t index = 0; index < token_count; index++)
	{
		local_out_tokens[index] = (char*)malloc(sizeof(char) * split_lengths[index]);
	}
	
	size_t token_offset = 0;
	for(size_t token_index = 0; token_index < token_count; token_index++)
	{
		for(size_t within_token_char_index = 0; within_token_char_index < split_lengths[token_index]; within_token_char_index++)
		{
			local_out_tokens[token_index][within_token_char_index] = stream[token_offset + within_token_char_index];
		}
		token_offset += (split_lengths[token_index] + 1);
	}

	*out_tokens = local_out_tokens;
	return token_count;
}

void calculate_token_type_counts(char** tokens, size_t* token_lengths, size_t num_tokens, struct Token_Counts* p_token_counts)
{
	size_t number_of_variables = 0;
	size_t number_of_lables = 0;
	size_t number_of_tokens_corresponding_to_variables = 0;

	for(size_t token_index = 0; token_index < num_tokens; token_index++)
	{
		char* token = tokens[token_index];
		if(token[token_lengths[token_index] - 1] == ':')
		{
			number_of_lables++;
			continue; }
		else if(token_lengths[token_index] != 3) {
			continue; }
		else if(token[0] == 'D' && token[1] == 'A' && token[2] == 'T'){
			number_of_variables++;
			number_of_tokens_corresponding_to_variables += 2;
			token_index++;
			if(token_index < num_tokens - 1)
			{
				if(tokens[token_index + 1][0] == '0' && tokens[token_index + 1][1] == 'x')
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

void split_tokens_by_type(char** tokens, size_t* token_lengths, size_t num_tokens, struct Variable* variables, struct Lable* lables, struct Token* instruction_tokens)
{
	byte location_counter = 0;
	size_t variable_index = 0;
	size_t lable_index = 0;
	size_t instruction_index = 0;

	for(size_t token_index = 0; token_index < num_tokens; token_index++)
	{
		char* token = tokens[token_index];
		size_t token_length = token_lengths[token_index];
		if(token[token_length - 1] == ':') {//Lable line
			struct Lable* p_lable = lables + lable_index;
			p_lable->length = token_length - 1;
			p_lable->name = (char*)malloc(sizeof(char) * token_length - 1);
		
			copy_char_array(token, p_lable->name, token_length - 1);
			
			p_lable->address = location_counter;
			lable_index++;
			continue; 
		} if(token_length == 3 && token[0] == 'D' && token[1] == 'A' && token[2] == 'T') {
			token_index++;
			struct Variable* p_variable = variables + variable_index;
			char* variable_name = tokens[token_index];
			token_length = token_lengths[token_index];
			p_variable->length = token_length;
			p_variable->name = (char*)malloc(sizeof(char) * token_length);
			
			copy_char_array(variable_name, p_variable->name, token_length);
			
			if(token_index < num_tokens - 1)
			{
				char* default_value = tokens[token_index + 1];
				if(default_value[0] == '0' && default_value[1] == 'x')
				{
					byte value = strtol(default_value, NULL, 16);
					p_variable->value = value;
					token_index++;
				}
			}
			variable_index++;
			continue;
		} else {
			struct Token* p_instruction_token = instruction_tokens + instruction_index;
			p_instruction_token->length = token_length;
			p_instruction_token->name = (char*)malloc(sizeof(char) * token_length);
			
			copy_char_array(token, p_instruction_token->name, token_length);
			
			instruction_index++;
			location_counter++; 
		}
	}	
}

void set_variable_addresses(struct Variable* variables, size_t number_of_variables)//Could put variables directly after instructions, however this is nice for now.
{
	for(size_t variable_index = 0; variable_index < number_of_variables; variable_index++)
	{
		variables[variable_index].address = MEMORY_SIZE - variable_index - 1;	
	}
}

void replace_lables_with_address(byte* program, struct Lable* lables, size_t number_of_lables, struct Token* instruction_tokens, size_t number_of_instruction_tokens)
{
	for(size_t token_index = 0; token_index < number_of_instruction_tokens; token_index++)
	{
		struct Token token = instruction_tokens[token_index];
		for(size_t lable_index = 0; lable_index < number_of_lables; lable_index++)
		{
			struct Lable lable = lables[lable_index];
			if(strcmp(lable.name, token.name) == 0) {
				program[token_index] = lable.address;
				break;
			}
		}
	}
}

void replace_variables_with_address(byte* program, struct Variable* variables, size_t number_of_variables, struct Token* instruction_tokens, size_t number_of_instruction_tokens)
{
	for(size_t token_index = 0; token_index < number_of_instruction_tokens; token_index++)
	{
		struct Token token = instruction_tokens[token_index];
		for(size_t variable_index = 0; variable_index < number_of_variables; variable_index++)
		{
			struct Variable variable = variables[variable_index];
			if(strcmp(variable.name, token.name) == 0) {
				program[token_index] = variable.address;
				break;
			}
		}
	}
}

void replace_instruction_with_opcode(byte* program, struct Token* instruction_tokens, size_t number_of_instruction_tokens)
{
	for(size_t token_index = 0; token_index < number_of_instruction_tokens; token_index++)
	{
		struct Token instruction_token = instruction_tokens[token_index];
		byte opcode = decode_opcode(instruction_token.name);
		//if token in pneumonics, convert.
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

void load_initial_variable_values(byte* program, struct Variable* variables, size_t number_of_variables)
{
	for(size_t variable_index = 0; variable_index < number_of_variables; variable_index++)
	{
		struct Variable variable = variables[variable_index];
		program[variable.address] = variable.value;
	}
}

void free_variables(struct Variable* variables, size_t count)
{
	for(size_t index = 0; index < count; index++)
	{
		free(variables[index].name);	
	}
	free(variables);
}

void free_lables(struct Lable* lables, size_t count)
{
	for(size_t index = 0; index < count; index++)
	{
		free(lables[index].name);
	}
	free(lables);
}

void free_tokens(struct Token* tokens, size_t count)
{
	for(size_t index = 0; index < count; index++)
	{
		free(tokens[index].name);
	}
	free(tokens);
}

int main(int argc, char* argv[])
{
	struct stat sb;
	int fd = open(argv[1], O_RDONLY);
        fstat(fd, &sb);           /* To obtain file size */
	int file_size = sb.st_size;
	char* address = (char*)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
	
	char* char_stream_with_comments = (char*)malloc(sizeof(char) * file_size);
	copy_char_array(address, char_stream_with_comments, file_size);
	munmap(address, file_size);
	close(fd);

	char* char_stream = remove_comments(char_stream_with_comments, file_size);
	free(char_stream_with_comments);
	replace(char_stream, '\t', ' ');
	replace(char_stream, '\n', ' ');
	
	char* new_char_stream = unpair_lables(char_stream);
	free(char_stream);
	
	char* token_char_stream_not_stripped = remove_repeated_whitespace(new_char_stream);
	free(new_char_stream);
	

	char* token_char_stream_stripped = strip(token_char_stream_not_stripped);
	free(token_char_stream_not_stripped);

	//Figure out how to turn this into the token structs 
	//----------------------------
	char** tokens;
	size_t* token_lengths;
       	size_t num_tokens = split_stream_by_char(token_char_stream_stripped, &tokens, &token_lengths, ' ');
	//----------------------------
	free(token_char_stream_stripped);

	struct Token_Counts token_counts;
	calculate_token_type_counts(tokens, token_lengths, num_tokens, &token_counts);

	struct Variable* variables = (struct Variable*)malloc(sizeof(struct Variable) * token_counts.number_of_variables);
	struct Lable* lables = (struct Lable*)malloc(sizeof(struct Lable) * token_counts.number_of_lables);
	struct Token* instruction_tokens = (struct Token*)malloc(sizeof(struct Token) * token_counts.number_of_instruction_tokens);

	split_tokens_by_type(tokens, token_lengths, num_tokens, variables, lables, instruction_tokens);
	
	free(tokens);
	free(token_lengths);
	
	set_variable_addresses(variables, token_counts.number_of_variables);	

	byte* program = (byte*)malloc(sizeof(byte) * MEMORY_SIZE);
	
	translate_to_machine_code(program, instruction_tokens, lables, variables, token_counts);
	
	load_initial_variable_values(program, variables, token_counts.number_of_variables);

	free_variables(variables, token_counts.number_of_variables);
	free_lables(lables, token_counts.number_of_lables);
	free_tokens(instruction_tokens, token_counts.number_of_instruction_tokens);

	FILE* p_file = fopen(argv[2], "wb");
	fwrite(program, sizeof(byte), MEMORY_SIZE, p_file);
	fclose(p_file);

	free(program);
	
	return 0;
}
