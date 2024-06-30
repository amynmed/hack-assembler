
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>


#include "assembler.h"




int main(int argc, char ** argv)
{
	
	if (argc < 2)
	{
		printf("No input files provided\n");
		return 0;
	}
	
	if (argc > 3)
	{
		printf("Provide one input file\n");
		return 0;
	}
	
	char * asm_file_name  	 = argv[1];
	
	FILE * asm_file 	 = NULL;
	
	char * period_ptr 	 = strchr(asm_file_name, '.');
	
	unsigned int stem_length = period_ptr - asm_file_name;
	
	char stem[MAX_STEM_SIZE];
	
	memcpy(stem, asm_file_name, stem_length);
	
	stem[stem_length] = '\0';
	
	asm_file = fopen(asm_file_name, "r");
	
	if(asm_file)
	{
		//printf("FILE IMPORTED\n");
		char * binary_file_name = strcat(stem, BINARY_OUTPUT_EXTENSION);
		
		//printf("CREATING BINARY FILE\n");
		FILE * binary_file = fopen(binary_file_name, "w");
		
		char line[MAX_LINE_SIZE];
		char * new_line = NULL;
		
		unsigned int line_count = 0;
		
		SYMBOL_TABLE * symbol_table = init_symbol_table();
		
		printf("CHECKING SIZE WHEN INIT : %d\n", symbol_table->size);
		
		while(fgets(line, sizeof line, asm_file) != NULL)
		{
			char * new_line = remove_whitespace(line);
			
			if(strstr(new_line, "//") == new_line) 
				continue;
			
			if(strlen(new_line) < 1) 
				continue;
			
			if(is_label(new_line))
			{
				unsigned int symbol_length = strlen(line + 1);
				
				char symbol[MAX_SYMBOL_LENGTH];
				
				memcpy(symbol, line + 1, symbol_length - 2);
				
				symbol[symbol_length - 2] = '\0';

				printf("FOUND SYMBOL %s AT LINE %d\n", symbol, line_count);
				
				add_symbol(symbol_table, symbol, line_count);
				
				continue;
			}
			printf("LINE : %s\n", new_line);
			printf("LINE NUMBER IN FIRST PASS %d\n", line_count);
			line_count++;
		}
		
		// Reset
		line_count = 0;
		rewind(asm_file);
		
		while(fgets(line, sizeof line, asm_file) != NULL)
		{
			new_line = remove_whitespace(line);

			if(strstr(new_line, "//") == new_line) 
				continue;
			
			if(strlen(new_line) < 1) 
				continue;
			
			if(is_label(new_line)) 
				continue;
			
			printf("TRANSLATING INSTRUCTION : %s AT LINE : %d\n", new_line, line_count);
			uint16_t binary = translate(new_line, symbol_table);
			
			char * binary_string = to_binary_string(binary);
			
			strcat(binary_string, "\n");
			
			fprintf(binary_file, binary_string);
			
			if (new_line != NULL) free(new_line);
			
			line_count++;
			
		}
		
		fclose(asm_file);
		fclose(binary_file);
		
		
	}
	
	else
	{
		printf("Invalid file path\n");
		return -1;
	}
	
	return 0;
	
}
//
//








bool is_label(char * s)
{
	return strchr(s, '(') == s && strchr(s, ')') == (s + strlen(s) - 1);
}

char * remove_whitespace(char * str) 
{
  unsigned int new_len = 0;

  for (int i = 0; str[i] != '\0'; i++) 
  {
    if (str[i] != ' ' && str[i] != '\n') new_len++;
  }

  char * new_string = malloc(new_len + 1);

  unsigned int i = 0, j = 0;
  
  while (str[i] != '\0') 
  {
    if (!isspace(str[i]) && str[i] != '\n' && str[i] != '\r')  new_string[j++] = str[i];
    i++;
  }

  new_string[j] = '\0';
  
  return new_string;
}
//


//
//
char * to_binary_string(uint16_t bit_instruction)
{
	char * binary_string = malloc(REGISTER_BITSIZE + 1);
	
	memset(binary_string, '0', REGISTER_BITSIZE + 1);
	
	binary_string[REGISTER_BITSIZE] = '\0';
	
	for(int i = 0; i < REGISTER_BITSIZE; i++)
	{
		if((1 << i & bit_instruction)) 
			binary_string[REGISTER_BITSIZE - i - 1] = '1';
	}
	
	return binary_string;
	
}
//
//

bool is_number(char * s)
{
	unsigned int s_length = strlen(s);
	for(int i = 0; i < s_length; i++)
	{
		if(!isdigit(s[i])) return false;
	}
	return true;
}

//

bool is_register(char * s)
{
	unsigned int number = atoi(s + 1);
	
	if(!(s[0] == 'R')) 	  	   return false;
	if(strlen(s + 1) > 2) 		   return false;
	if(!is_number(s + 1))  	           return false;
	if(!(number >= 0 && number <= 15)) return false;
	
	return true;
}

// Symbol table-----------------------
//

SYMBOL_TABLE * init_symbol_table()
{
	
	unsigned int capacity = SYMBOL_TABLE_CAPACITY;
	
	SYMBOL_TABLE * st = (SYMBOL_TABLE *) malloc(sizeof(SYMBOL_TABLE));
	
	*st = (SYMBOL_TABLE) {0, capacity, 16, malloc(capacity * sizeof(char *)), malloc(capacity * sizeof(unsigned int))};
	
	return st;
	
}

unsigned int * symbol_value(SYMBOL_TABLE * st, char * symbol)
{
	if(!symbol) return NULL;
	
	for(int i = 0; i < st->size; i++)
	{
		unsigned int compare_val = strcmp(st->symbols[i], symbol);

		if (compare_val == 0)
		{
			return &st->values[i];
		}
			
	}
	
	return NULL;
}

unsigned int add_symbol(SYMBOL_TABLE * st, char * symbol, int line_number)
{
	
	if (st->size >= st->capacity)
	{
		st->capacity *= 2;
		st->symbols   =        (char **) realloc(st->symbols, st->capacity);
		st-> values   = (unsigned int *) realloc(st->values , st->capacity);
	}
	
	char * _symbol = malloc(strlen(symbol) + 1);
	
	memcpy(_symbol, symbol, strlen(symbol));
	
	_symbol[strlen(symbol)] = '\0';
	
	if(line_number != -1) st->values[st->size ++] = line_number;
		
	else st-> values[st->size ++] = st->symbol_starting_index ++;
		
	st->symbols[st->size - 1] = _symbol;
	 
	return st->values[st->size - 1];
	
	
}


uint16_t jump_bits(char * jump_instruction)
{
	enum JUMP_CONDITION jump_instruction_code = NO_JMP;
			
	     if (strcmp(jump_instruction, "JMP") == 0) jump_instruction_code = JMP;
	else if (strcmp(jump_instruction, "JLT") == 0) jump_instruction_code = JLT;
	else if (strcmp(jump_instruction, "JGT") == 0) jump_instruction_code = JGT;
	else if (strcmp(jump_instruction, "JGE") == 0) jump_instruction_code = JGE;
	else if (strcmp(jump_instruction, "JLE") == 0) jump_instruction_code = JLE;
	else if (strcmp(jump_instruction, "JEQ") == 0) jump_instruction_code = JEQ;
	else if (strcmp(jump_instruction, "JNE") == 0) jump_instruction_code = JNE;
	
	// invalid jump
	else
	{
		printf("%s\tInvalid jump instruction\n", jump_instruction);
		return -1;
	}
	
	return jump_instruction_code;
}

uint16_t destination_bits(char *destination_string)
{
	enum DENSTINATION destination_bits;
			
	     if (strcmp(destination_string, "A")   == 0)  destination_bits =   A;
	else if (strcmp(destination_string, "M")   == 0)  destination_bits =   M;
	else if (strcmp(destination_string, "D")   == 0)  destination_bits =   D;
	else if (strcmp(destination_string, "AD")  == 0)  destination_bits =  AD;
	else if (strcmp(destination_string, "MD")  == 0)  destination_bits =  MD;
	else if (strcmp(destination_string, "AM")  == 0)  destination_bits =  AM;
	else if (strcmp(destination_string, "AMD") == 0)  destination_bits = AMD;
	
	// invalid destination
	else
		return -1;
	
	return destination_bits;
}

uint16_t comp_bits(char * comp_string)
{
		uint16_t bit_instruction = 0;
		
		char a_bit = 0;
		enum COMP comp_bits;
		
		if (strcmp(comp_string,   "0") == 0) 					    comp_bits = COMP_ZERO;
		else if (strcmp(comp_string,   "1") == 0) 				    comp_bits = COMP_ONE;
		else if (strcmp(comp_string,  "-1") == 0) 				    comp_bits = COMP_MINUS_ONE;
		else if (strcmp(comp_string,   "D") == 0) 				    comp_bits = COMP_D;
		else if (strcmp(comp_string,  "!D") == 0) 				    comp_bits = COMP_NOT_D;
		else if (strcmp(comp_string, "D+1") == 0) 				    comp_bits = COMP_D_PLUS_ONE;
		else if (strcmp(comp_string, "D-1") == 0) 				    comp_bits = COMP_D_MINUS_ONE;
		else if (strcmp(comp_string,  "!A") == 0 | strcmp(comp_string,  "!M") == 0) comp_bits = COMP_NOT_A_OR_NOT_M;
		else if (strcmp(comp_string,   "A") == 0 | strcmp(comp_string,   "M") == 0) comp_bits = COMP_A_OR_M;
		else if (strcmp(comp_string,  "-A") == 0 | strcmp(comp_string,  "-M") == 0) comp_bits = COMP_MINUS_A_OR_MINUS_M;
		else if (strcmp(comp_string, "A+1") == 0 | strcmp(comp_string, "M+1") == 0) comp_bits = COMP_A_PLUS_ONE_OR_M_PLUS_ONE;
		else if (strcmp(comp_string, "A-1") == 0 | strcmp(comp_string, "M-1") == 0) comp_bits = COMP_A_MINUS_ONE_OR_M_MINUS_ONE;
		else if (strcmp(comp_string, "D+A") == 0 | strcmp(comp_string, "D+M") == 0) comp_bits = COMP_D_PLUS_A_OR_D_PLUS_M;
		else if (strcmp(comp_string, "D-A") == 0 | strcmp(comp_string, "D-M") == 0) comp_bits = COMP_D_MINUS_A_OR_D_MINUS_M;
		else if (strcmp(comp_string, "A-D") == 0 | strcmp(comp_string, "M-D") == 0) comp_bits = COMP_A_MINUS_D_OR_M_MINUS_D;
		else if (strcmp(comp_string, "D&A") == 0 | strcmp(comp_string, "D&M") == 0) comp_bits = COMP_D_AND_A_OR_D_AND_M;
		else if (strcmp(comp_string, "D|A") == 0 | strcmp(comp_string, "D|M") == 0) comp_bits = COMP_D_OR_A_OR_D_OR_M;
		
		if (strchr(comp_string, 'M')) a_bit = 1;
		
		bit_instruction |= comp_bits << JUMP_INSTRUCTION_SIZE + DESTINATION_INSTRUCTION_SIZE;
		bit_instruction |= a_bit     << JUMP_INSTRUCTION_SIZE + DESTINATION_INSTRUCTION_SIZE + COMP_INSTRUCTION_SIZE;
		
		return bit_instruction;
}


uint16_t translate(char *instruction, SYMBOL_TABLE *st)
{

	uint16_t bit_instruction = 0;
	
	// A instruction
	if(a_instruction(instruction))
	{
		size_t len_address_string = strlen(instruction);
		
		char address_string[MAX_SYMBOL_LENGTH];
		memcpy(address_string, instruction + 1, len_address_string * sizeof(char));	
		address_string[len_address_string] = '\0';
		
		uint16_t address;

		if (is_number(address_string))
		{
			address = atoi(address_string);
			
			for (int i = 0; i < MSB_INDEX; i++)
			{
				// checking if bit at i is 1
				if(address & 1 << i) bit_instruction |= 1 << i;
			}
			
			
			return bit_instruction;
		}
		
		else
		{
			// Check R symbol
			if(is_register(address_string))
			{
				
				char register_number_string [MAX_SYMBOL_LENGTH];
				memcpy(register_number_string, address_string + 1, len_address_string);
				register_number_string[len_address_string] = '\0';
				
				uint16_t register_number = atoi(register_number_string);

				printf("R ADDRESS: %s | NUMBER : %d\n", address_string, register_number);
				
				if(register_number >= 0)
					bit_instruction |= register_number;
				
				return bit_instruction;
			}
			
			else
			{
				if(strcmp(address_string, "SP") 	 == 0) return SP;
				if(strcmp(address_string, "LCL") 	 == 0) return LCL;
				if(strcmp(address_string, "ARG") 	 == 0) return ARG;
				if(strcmp(address_string, "THIS") 	 == 0) return THIS;
				if(strcmp(address_string, "THAT")	 == 0) return THAT;
				if(strcmp(address_string, "SCREEN")      == 0) return SCREEN;
				if(strcmp(address_string, "KBD")	 == 0) return KBD;

				unsigned int * val  = NULL;
				val = symbol_value(st, address_string);
				
				if (val == NULL) bit_instruction = add_symbol(st, address_string, -1);
					
				else bit_instruction = *val;
					
				return bit_instruction;
			}
		}

		return bit_instruction;
		//
	}
	
	// C instruction
	else
	{
		
		bit_instruction |= 1 << 15 | 1 << 14 | 1 << 13;
		
		char * semi_colon_ptr = strchr(instruction, SEMI_COLON);
		char * equal_sign_ptr = strchr(instruction, EQUAL_SIGN);
		
		// Illegal
		if(!semi_colon_ptr && !equal_sign_ptr) return -1;
		
		else if(semi_colon_ptr && !equal_sign_ptr)
		{
			uint16_t pos = semi_colon_ptr - instruction ;
			
			//
			size_t comp_length = pos;
			char comp_string [MAX_SYMBOL_LENGTH];
			memcpy(comp_string, instruction, comp_length);
			comp_string[comp_length] = '\0';
			
			uint16_t comp = comp_bits(comp_string);
			
			bit_instruction |= comp;
			
			if((strlen(instruction) - (pos + 1)) != JUMP_INSTRUCTION_SIZE)
			{
				printf("%s\tInvalid instruction\n", instruction);
				return -1;
			}
			
			char jump_instruction[JUMP_INSTRUCTION_SIZE + 1];
			
			memcpy(jump_instruction, semi_colon_ptr + 1, JUMP_INSTRUCTION_SIZE);
			
			jump_instruction[JUMP_INSTRUCTION_SIZE] = '\0';
			
			uint16_t jump_instruction_code = jump_bits(jump_instruction);
			
			bit_instruction |= jump_instruction_code;
			
			return bit_instruction;

		}
		
		
		else if (!semi_colon_ptr && equal_sign_ptr)
		{
			uint16_t pos = equal_sign_ptr - instruction;
			
			char destination_string [MAX_SYMBOL_LENGTH];
			memcpy(destination_string, instruction, pos);
			destination_string[pos] = '\0';
			
			uint16_t dest_bits = destination_bits(destination_string);
			
			if (dest_bits) bit_instruction |= dest_bits << JUMP_INSTRUCTION_SIZE;
			
			else
			{
				printf("INVALID DESTINATION IN INSTRUCTION : %s\n", instruction);
				return -1;
			}
			
			size_t comp_length = strlen(equal_sign_ptr) + 1;
			char comp_string [MAX_SYMBOL_LENGTH];
			memcpy(comp_string, equal_sign_ptr + 1, comp_length);
			comp_string[comp_length ] = '\0';
			
			uint16_t comp = comp_bits(comp_string);
			
			bit_instruction |= comp;
			
			return bit_instruction;
		}
		
		else
		{
			// Getting destination bits
			uint16_t dest_length = equal_sign_ptr - instruction;
			
			char destination_string [MAX_SYMBOL_LENGTH];
			memcpy(destination_string, instruction, dest_length);
			destination_string[dest_length] = '\0';
			
			uint16_t dest_bits = destination_bits(destination_string);
			
			if (dest_bits)
				bit_instruction |= dest_bits << JUMP_INSTRUCTION_SIZE;
			else
			{
				printf("INVALID DESTINATION (%s) IN INSTRUCTION : %s\n", destination_string, instruction);
				return -1;
			}
			
			uint16_t comp_length = semi_colon_ptr - equal_sign_ptr ;
			
			char comp_string[MAX_SYMBOL_LENGTH];
			memcpy(comp_string, equal_sign_ptr + 1, comp_length);
			
			comp_string[comp_length] = '\0';
			
			uint16_t comp = comp_bits(comp_string);
			
			if (comp_bits >= 0)
			{
				bit_instruction |= comp;
				return bit_instruction;
			}	
			
			else
			{
				printf("INVALID COMP : %s\n", comp);
				return -1;
			}
			
			
			char jump_instruction[JUMP_INSTRUCTION_SIZE + 1];
			
			memcpy(jump_instruction, semi_colon_ptr + 1, JUMP_INSTRUCTION_SIZE);
			
			jump_instruction[JUMP_INSTRUCTION_SIZE] = '\0';
			
			uint16_t jump_instruction_code = jump_bits(jump_instruction);
			
			bit_instruction |= jump_instruction_code;
			
		}
		
		free(semi_colon_ptr);
		free(equal_sign_ptr);
		
		return bit_instruction;
		
	}
}