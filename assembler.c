
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>


#define BINARY_OUTPUT_EXTENSION ".hack"

#define MAX_STEM_SIZE	  		1 << 10
#define MAX_LINE_SIZE 	  	    1 << 12
#define MAX_SYMBOL_LENGTH 		1 << 10

#define REGISTER_BITSIZE 			 16

#define MSB_INDEX 					 15
#define JUMP_INSTRUCTION_SIZE 		  3
#define DESTINATION_INSTRUCTION_SIZE  3
#define COMP_INSTRUCTION_SIZE 		  6


#define SEMI_COLON ';'
#define EQUAL_SIGN '='

#define a_instruction(s) s[0] == '@'


// to refactor
char * remove_whitespace(char * str) 
{
  unsigned int new_len = 0;

  for (int i = 0; str[i] != '\0'; i++) 
  {
    if (str[i] != ' ' && str[i] != '\n') 
	{
      new_len++;
    }
  }

  char * new_string = malloc(new_len + 1);

  unsigned int i = 0;
  unsigned int j = 0;
  
  while (str[i] != '\0') 
  {
    if (!isspace(str[i]) && str[i] != '\n') 
	{
      new_string[j++] = str[i];
    }
	
    i++;
  }

  new_string[j] = '\0';
  
  //printf("AFTER WHITEPSPACE REMOVAL : %s\n", new_string);

  return new_string;
}
//


// BIN TO STRING
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
	
	if(!(s[0] == 'R')) 	  			   return false;
	if(strlen(s + 1) > 2) 			   return false;
	if(!is_number(s + 1))  			   return false;
	if(!(number >= 0 && number <= 15)) return false;
	
	return true;
}

// Symbol table-----------------------
//

typedef struct SYMBOL_TABLE SYMBOL_TABLE;

struct SYMBOL_TABLE
{
	unsigned int 	size;
	unsigned int 	capacity;
	unsigned int 	symbol_starting_index; 
	
	       char **  symbols;
	unsigned int *  values;
	
};

SYMBOL_TABLE * init_symbol_table()
{
	
	unsigned int capacity = 1 << 12;
	
	SYMBOL_TABLE * st = (SYMBOL_TABLE *) malloc(sizeof(SYMBOL_TABLE));
	
	*st = (SYMBOL_TABLE) {0, capacity, 16, malloc(capacity * sizeof(char *)), malloc(capacity * sizeof(unsigned int))};
	
	//printf("TABLE SIZE is %d\n", st->size);
	
	return st;
	
}

unsigned int * symbol_value(SYMBOL_TABLE * st, char * symbol)
{
	//printf("CURRENT TABLE SIZE : %d\n", st->size);
	
	if(!symbol) return NULL;
	
	for(int i = 0; i < st->size; i++)
	{
		//printf("SYMBOL IN TABLE %s & %s WITH VALUE %d: \n", st->symbols[i], symbol, st->values[i]);
		//if (!st->symbols[i]) continue;
		
		//printf("COMPARING SYMBOL %40s WITH SYMBOL %s\n", st->symbols[i], symbol);
		unsigned int compare_val = strcmp(st->symbols[i], symbol);
		//printf("COMPARE VAL : %d\n", compare_val);
		if (compare_val == 0)
		{
			//printf("SYMBOL FOUND : %40s | WITH VALUE : %d at index %d \n", symbol, st->values[i], i);
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
		
		st->symbols   =    	   (char **) realloc(st->symbols, st->capacity);
		st-> values   = (unsigned int *) realloc(st->values , st->capacity);
	}
	
	char * _symbol = malloc(strlen(symbol) + 1);
	
	memcpy(_symbol, symbol, strlen(symbol));
	
	_symbol[strlen(symbol)] = '\0';
	
	//printf("COPIED SYMBOL %s\n", _symbol);
	
	
	if(line_number != -1)
	{
		//printf("LINE NUMBER GIVEN IN ADD SYMBOL FUNCTION FOR SYMBOL %s : %d _________________________________\n", _symbol, line_number);
		st->values[st->size] = line_number;
		st->size ++;
	}
		
	else
	{
		//st->symbol_starting_index += 1;
		st-> values[st->size ++] = st->symbol_starting_index ++;
		//printf("CURRENT TABLE SIZE : %d | NEW SYMBOL VALUE : %d\n", st->size, st->symbol_starting_index);
		//st->size ++;
		
	}
	
	st->symbols[st->size - 1] = _symbol;
	 
	return st->values[st->size - 1];
	
	
}

//
// Symbol table-----------------------

enum COMP
{
	COMP_ZERO 					   	   =  0b00101010,
	COMP_ONE  					   	   =  0b00111111,
	COMP_MINUS_ONE 				       =  0b00111010,
	COMP_D 						   	   =  0b00001100,
	COMP_A_OR_M 					   =  0b00110000,
	COMP_NOT_D 					       =  0b00001101,
	COMP_NOT_A_OR_NOT_M 			   =  0b00110001,
	COMP_MINUS_D 				       =  0b00001111,
	COMP_MINUS_A_OR_MINUS_M 		   =  0b00110011,
	COMP_D_PLUS_ONE 				   =  0b00011111,
	COMP_A_PLUS_ONE_OR_M_PLUS_ONE      =  0b00110111,
	COMP_D_MINUS_ONE 			   	   =  0b00001110,
	COMP_A_MINUS_ONE_OR_M_MINUS_ONE    =  0b00110010,
	COMP_D_PLUS_A_OR_D_PLUS_M 	   	   =  0b00000010,
	COMP_D_MINUS_A_OR_D_MINUS_M 	   =  0b00010011,
	COMP_A_MINUS_D_OR_M_MINUS_D        =  0b00000111,
	COMP_D_AND_A_OR_D_AND_M 		   =  0b00000000,
	COMP_D_OR_A_OR_D_OR_M 		       =  0b00010101
};

enum PREDEFINED_SYMBOL
{
	    SP = 0,
	   LCL = 1,
	   ARG = 2,
	  THIS = 3,
	  THAT = 4,
	SCREEN = 1 << 14,
	   KBD = 1 << 14 | 1 << 13
};

enum DENSTINATION
{
	NO_DEST = 0x00,
	M   	= 0x01,
	D   	= 0x02,
	MD 		= 0x03,
	A   	= 0x04,
	AM  	= 0x05,
	AD  	= 0x06,
	AMD 	= 0x07
};

enum JUMP_CONDITION
{
	NO_JMP = 0x00,
	   JGT = 0x01,
	   JLT = 0x04,
	   JLE = 0x06,
  	   JGE = 0x03,
	   JEQ = 0x02,
	   JNE = 0x05,
	   JMP = 0x07
	
};

uint16_t jump_bits(char * jump_instruction)
{
	enum JUMP_CONDITION jump_instruction_code = NO_JMP;
			
	// getting code:
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
	
	//printf("JUMP INSTRUCTION : %s, JUMP_CODE : %d, BINARY JUMP CODE : %s\n", jump_instruction, jump_instruction_code, to_binary_string(jump_instruction_code));
	
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
		
		//printf("COMP STRING : %s\n", comp_string);
		
			 if (strcmp(comp_string,   "0") == 0) 						      		comp_bits = COMP_ZERO;
		else if (strcmp(comp_string,   "1") == 0) 						      		comp_bits = COMP_ONE;
		else if (strcmp(comp_string,  "-1") == 0) 							  		comp_bits = COMP_MINUS_ONE;
		else if (strcmp(comp_string,   "D") == 0) 						      		comp_bits = COMP_D;
		else if (strcmp(comp_string,  "!D") == 0) 							  		comp_bits = COMP_NOT_D;
		else if (strcmp(comp_string, "D+1") == 0) 						      		comp_bits = COMP_D_PLUS_ONE;
		else if (strcmp(comp_string, "D-1") == 0) 							  		comp_bits = COMP_D_MINUS_ONE;
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
		
		//
		if (strchr(comp_string, 'M')) a_bit = 1;
		
		//
		//printf("(comp_bits_function) COMP_BITS : %s\n", to_binary_string(comp_bits));
		
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
		
		// dec to bin
		size_t len_address_string = strlen(instruction);
		
		char address_string[MAX_SYMBOL_LENGTH];
		memcpy(address_string, instruction + 1, len_address_string * sizeof(char));	
		address_string[len_address_string] = '\0';
		
		uint16_t address;
		
		//printf("ADDRESS STRING : @%s _______________________________________________________________\n", address_string, address);
		
		// if address is a number
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
		
		// *TODO : must check for user defined symbols
		/*
		*
		**/
		else
		{
			// Check R symbol
			if(is_register(address_string))
			{
				char register_number_string [MAX_SYMBOL_LENGTH];
				memcpy(register_number_string, address_string + 1, len_address_string);
				register_number_string[len_address_string] = '\0';
				
				uint16_t register_number = atoi(register_number_string);
				
				//printf("REGISTER NUMBER STRING : %s\n", register_number_string);
				//printf("REGISTER NUMBER : 		 %d\n", register_number);
				
				
				if(register_number)
					bit_instruction |= register_number;
				
				//printf("BIT INSTRUCTION OF R ADDRESS : %s\n", to_binary_string(bit_instruction));

				
				return bit_instruction;
			}
			
			// *TODO : get symbols:
			else
			{
				//printf("A INSTRUCTION | LOOKING FOR SYMBOL#################################### \n");
				
				if(strcmp(address_string, "SP") 	 == 0) return SP;
				if(strcmp(address_string, "LCL") 	 == 0) return LCL;
				if(strcmp(address_string, "ARG") 	 == 0) return ARG;
				if(strcmp(address_string, "THIS") 	 == 0) return THIS;
				if(strcmp(address_string, "THAT")	 == 0) return THAT;
				if(strcmp(address_string, "SCREEN")  == 0) return SCREEN;
				if(strcmp(address_string, "KBD")	 == 0) return KBD;
				
				
				// Check if symbol exits else include it
				//printf("CHECKING IF ADDRESS SYMBOL %s EXISTS AND HAS VALUE IN ST ...\n", address_string); 
				
				unsigned int * val  = NULL;
				val = symbol_value(st, address_string);
				
				if(val == NULL)
				{
					//printf("ADDING ADDRESS SYMBOL (%s) TO ST ...\n", address_string);
					bit_instruction =   add_symbol(st, address_string, -1);
				}
					
				else 
				{
					//printf("ADDRESS SYMBOL (%s) ALREADY EXIST WITH VALUE (%d)\n", address_string, *val);
					bit_instruction = *val;
				}
					
				//printf("ADDRESS BIT INSTRUCTION : %s\n", to_binary_string(bit_instruction));
				return bit_instruction;
			}
			
			return bit_instruction;
		}
		//
		//
	}
	
	// C instruction
	else
	{
		
		bit_instruction |= 1 << 15 | 1 << 14 | 1 << 13;
		
		// jump condition parse
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
			
			printf("COMP STRING : %s, COMP BITS : %s, INSTRUCTION : %s\n", comp_string, to_binary_string(comp), instruction);
			
			bit_instruction |= comp;
			
			//
			
			
			if((strlen(instruction) - (pos + 1)) != JUMP_INSTRUCTION_SIZE)
			{
				printf("%s\tInvalid instruction\n", instruction);
				return -1;
			}
			
			
			char jump_instruction[JUMP_INSTRUCTION_SIZE + 1];
			
			memcpy(jump_instruction, semi_colon_ptr + 1, JUMP_INSTRUCTION_SIZE);
			
			jump_instruction[JUMP_INSTRUCTION_SIZE] = '\0';
			
			//printf("JUMP INSTRUCTION : %s\n", jump_instruction);
			
			uint16_t jump_instruction_code = jump_bits(jump_instruction);
			
			// copying jump ins into bit ins:
			bit_instruction |= jump_instruction_code;
			
			return bit_instruction;
			

		}
		
		
		else if (!semi_colon_ptr && equal_sign_ptr)
		{
			// Getting destination bits
			uint16_t pos = equal_sign_ptr - instruction;
			
			char destination_string [MAX_SYMBOL_LENGTH];
			memcpy(destination_string, instruction, pos);
			destination_string[pos] = '\0';
			
			uint16_t dest_bits = destination_bits(destination_string);
			
			if (dest_bits)
				bit_instruction |= dest_bits << JUMP_INSTRUCTION_SIZE;
			
			// invalid destination
			else
			{
				printf("INVALID DESTINATION IN INSTRUCTION : %s\n", instruction);
				return -1;
			}
				
			
			// Checking comp
			size_t comp_length = strlen(equal_sign_ptr) + 1;
			char comp_string [MAX_SYMBOL_LENGTH];
			memcpy(comp_string, equal_sign_ptr + 1, comp_length);
			comp_string[comp_length ] = '\0';
			
			uint16_t comp = comp_bits(comp_string);
			
			//printf("COMP STRING : %s, COMP BITS : %s, INSTRUCTION : %s\n", comp_string, to_binary_string(comp), instruction);
			
			bit_instruction |= comp;
			
			return bit_instruction;
			
			
		}
		
		
		// = & ; are present
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
			// invalid destination
			else
			{
				printf("INVALID DESTINATION (%s) IN INSTRUCTION : %s\n", destination_string, instruction);
				return -1;
			}
			
			// Getting comp bits
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
			
			// invalid destination
			else
			{
				printf("INVALID COMP : %s\n", comp);
				return -1;
			}
			
			
			// Jump
			char jump_instruction[JUMP_INSTRUCTION_SIZE + 1];
			
			memcpy(jump_instruction, semi_colon_ptr + 1, JUMP_INSTRUCTION_SIZE);
			
			jump_instruction[JUMP_INSTRUCTION_SIZE] = '\0';
			
			uint16_t jump_instruction_code = jump_bits(jump_instruction);
			
			// copying jump ins into bit ins:
			bit_instruction |= jump_instruction_code;
			
			
				 
			
			
		}
		
		free(semi_colon_ptr);
		free(equal_sign_ptr);
		
		return bit_instruction;
		
	}
}



int main(int argc, char ** argv)
{
	
	if (argc < 2)
	{
		printf("No input files provided\n");
		return 0;
	}
	
	if (argc > 3)
	{
		printf("Only one input file\n");
		return 0;
	}
	
	char * asm_file_name = argv[1];
	
	// Load asm files and parse:
	FILE * asm_file = NULL;
	
	char * period_ptr = strchr(asm_file_name, '.');
	
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
		
		// First Pass: parsing labels:
		//printf("FIRST PASS \n");
		while(fgets(line, sizeof line, asm_file) != NULL)
		{
			char * new_line = remove_whitespace(line);
			
			if(strstr(line, "//") == line)
			{
				//printf("FOUND COMMENT\n");
				continue;
			}
			
			
			new_line = remove_whitespace(line);
			
			if(strlen(new_line) <= 1) 
				continue;
			
			
			if(strchr(new_line, '(') == new_line && strchr(new_line, ')') == (new_line + strlen(new_line) - 1))
			{
				//printf("ADDING LABEL SYMBOL : %s##############################\n", new_line + 1);
				
				unsigned int symbol_length = strlen(line + 1);
				
				char symbol[MAX_SYMBOL_LENGTH];
				
				memcpy(symbol, line + 1, symbol_length - 2);
				
				symbol[symbol_length - 2] = '\0';
				
				add_symbol(symbol_table, symbol, line_count);
				
				//printf("LINE COUNT : %d, LABEL , LABEL VALUE : %s , %d\n##############################\n", line_count, symbol, *symbol_value(symbol_table, symbol));
				
				continue;
			}
			
			line_count++;
		}
		
		//printf("================FIRST PASS DONE=================\n==================================================\n");
		
		//for(int i = 0; i < symbol_table->size; i++)
		//	printf("AT index = %d, SYMBOL = %s, VALUE = %d\n", i, symbol_table->symbols[i], symbol_table->values[i]);
		
		// Reset
		line_count = 0;
		rewind(asm_file);
		
		//printf("SECOND PASS ______________________________________________________________________________________________\n");
		while(fgets(line, sizeof line, asm_file) != NULL)
		{
			//printf("CHECKING LINE : %d\n", line_count);
			
			
			
			if(strstr(line, "//") == line)
			{
				//printf("FOUND COMMENT\n");
				continue;
			}
			
			
			new_line = remove_whitespace(line);
			
			if(strlen(new_line) <= 1) 
				continue;
			
			//printf("STRING LENGTH : %d\n", strlen(new_line));
			
			if(strchr(new_line, '(') == new_line && strchr(new_line, ')') == (new_line + strlen(new_line) - 1)) 
				continue;
			
			uint16_t binary = translate(new_line, symbol_table);
			
			char * binary_string = to_binary_string(binary);
			
			
			
			//printf("===================\nLINE : %s | BINARY STRING : %s\n===================\n", new_line, binary_string);
			
			strcat(binary_string, "\n");
			
			fprintf(binary_file, binary_string);
			
			if (new_line != NULL) 	   free(new_line);
			//if (binary_string != NULL) free(binary_string);
			
			line_count++;
			
		}
		
		//printf("CLOSING FILES\n");
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