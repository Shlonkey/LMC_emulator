#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<unistd.h>
#include<stdint.h>

#define MEMORY_SIZE 0xFF

typedef uint8_t byte;

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
	size_t whitespace_repeats = 0;
	size_t char_count = 0;
	for(size_t index = 0; char_stream[index] != '\0'; index++)
	{
		whitespace_repeats += (char_stream[index] == ' ' && char_stream[index + 1] == ' ' ? 1 : 0);
		char_count++;
	}
	size_t new_length = char_count - whitespace_repeats;
	char* new_char_stream = (char*)malloc(sizeof(char) * new_length);
	size_t offset = 0;
	for(size_t index = 0; char_stream[index] != '\0'; index++)
	{
		if(char_stream[index] == ' ' && char_stream[index + 1] == ' ')
			offset++;	
		else
			new_char_stream[index - offset] = char_stream[index];
	}
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
	//We have now read the file in to a char* we want to remove comments
	//From each ';' to the next \n replace chars with ' '. also replace any '\t' with ' '

	char* char_stream = remove_comments(char_stream_with_comments, file_size);
	free(char_stream_with_comments);

	replace(char_stream, '\t', ' ');
	replace(char_stream, '\n', ' ');
	//Now make sure labels are seperated from the following Opcode i.e. OUT:HLT -> OUT: HLT.
	char* new_char_stream = unpair_lables(char_stream);
	free(char_stream);

	char* token_char_stream_not_stripped = remove_repeated_whitespace(new_char_stream);
	free(new_char_stream);
	
	char* token_char_stream_stripped = strip(token_char_stream_not_stripped);
	free(token_char_stream_not_stripped);
	
	char** tokens;
	size_t* token_lengths;
       	size_t num_tokens = split_stream_by_char(token_char_stream_stripped, &tokens, &token_lengths, ' ');

	free(token_char_stream_stripped);
	//Now all tokens (opcodes, constants and labels are seperated by into tokens) we need to get variable names and defult values, and lable jump addresses as well as converting PNEUMONICS to machinecode. and finally assiging variable addresses.

	size_t number_of_variables = 0;
	size_t number_of_tokens_corresponding_to_variables = 0;
	size_t number_of_lables = 0;
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
	size_t number_of_non_variable_or_lable_tokens = num_tokens - number_of_tokens_corresponding_to_variables - number_of_lables;
	size_t* variable_name_lengths = (size_t*)malloc(sizeof(size_t) * number_of_variables); 
	char** variable_names = (char**)malloc(sizeof(char*) * number_of_variables); 
	byte* variable_values = (byte*)malloc(sizeof(byte) * number_of_variables);
	byte* variable_addresses = (byte*)malloc(sizeof(byte) * number_of_variables);

	size_t* lable_name_lengths = (size_t*)malloc(sizeof(size_t) * number_of_lables); 
	char** lable_names = (char**)malloc(sizeof(char*) * number_of_lables);
	byte* lable_addresses = (byte*)malloc(sizeof(byte) * number_of_lables);

	size_t* non_variable_or_lable_token_lengths = (size_t*)malloc(sizeof(size_t) * number_of_non_variable_or_lable_tokens); 
	char** non_variable_or_lable_tokens = (char**)malloc(sizeof(char*) * number_of_non_variable_or_lable_tokens);
	//Now we have allocated memory for all the token data, split the tokens into thier representative info.
	//I.e. extract data from lables and dat tokens and determine default values.

	byte location_counter = 0;
	size_t variable_index = 0;
	size_t lable_index = 0;
	size_t non_variable_or_lable_index = 0;
	for(size_t token_index = 0; token_index < num_tokens; token_index++)
	{
		char* token = tokens[token_index];
		size_t token_length = token_lengths[token_index];
		if(token[token_length - 1] == ':') {//Lable line
			lable_name_lengths[lable_index] = token_length - 1;
			lable_names[lable_index] = (char*)malloc(sizeof(char) * token_length - 1);
			for(size_t i = 0; i < token_length - 1; i++)
			{
				lable_names[lable_index][i] = token[i];
			}
			lable_addresses[lable_index] = location_counter;
			lable_index++;
			continue; 
		} if(token_length == 3 && token[0] == 'D' && token[1] == 'A' && token[2] == 'T') {
			token_index++;
			char* variable_name = tokens[token_index];
			token_length = token_lengths[token_index];
			variable_name_lengths[variable_index] = token_length;
			variable_names[variable_index] = (char*)malloc(sizeof(char) * token_length);
			for(size_t i = 0; i < token_length; i++)
			{
				variable_names[variable_index][i] = variable_name[i];
			}
			if(token_index < num_tokens - 1)
			{
				char* default_value = tokens[token_index + 1];
				if(default_value[0] == '0' && default_value[1] == 'x')
				{
					byte value = strtol(default_value, NULL, 16);
					variable_values[variable_index] = value;
					token_index++;
				}
			}
			variable_index++;
			continue;
		}
		non_variable_or_lable_token_lengths[non_variable_or_lable_index] = token_length;
		non_variable_or_lable_tokens[non_variable_or_lable_index] = (char*)malloc(sizeof(char) * token_length);
		for(size_t i = 0; i < token_length; i++)
		{
			non_variable_or_lable_tokens[non_variable_or_lable_index][i] = token[i];
		}
		non_variable_or_lable_index++;
		location_counter++;
	}	
	free(tokens);
	free(token_lengths);
	//We have now split the file into instruction_tokens, variables and lables.
	//Now for each occurance of lable, replace with its address.

	byte* program = (byte*)malloc(sizeof(byte) * MEMORY_SIZE);
	for(size_t token_index = 0; token_index < number_of_non_variable_or_lable_tokens; token_index++)
	{
		char* token = non_variable_or_lable_tokens[token_index];
		//if token in pneumonics, convert.
		//else if token is lable replace with address byte
		//else if token is variable, replace with address byte
		//else if token is '0xAB' replace with 0xAB
		//finally set variable address value to defaults.
	}

	return 0;
}
