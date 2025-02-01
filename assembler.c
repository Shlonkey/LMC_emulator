#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<unistd.h>

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

int main(int argc, char* argv[])
{
	struct stat sb;
	int fd = open("rom.asm", O_RDONLY);
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

	char* token_char_stream = remove_repeated_whitespace(new_char_stream);

	//Now all tokens (opcodes, constants and labels are seperated) we need to get variable names and defult values, and lable jump addresses as well as converting PNEUMONICS to machinecode. and finally assiging variable addresses.

	printf("%s", token_char_stream);
	return 0;
}
