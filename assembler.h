#ifndef ASSEMBLER
#define ASSEMBLER


#include <stdint.h>
#include <stdbool.h>



#define BINARY_OUTPUT_EXTENSION  ".hack"
#define BINARY_OUTPUT_FOLDER      "hack"

#define SYMBOL_TABLE_CAPACITY   1 << 12
#define MAX_STEM_SIZE	  	1 << 10
#define MAX_LINE_SIZE 	  	1 << 12
#define MAX_SYMBOL_LENGTH 	1 << 10

#define REGISTER_BITSIZE 	     16

#define MSB_INDEX 		     15
#define JUMP_INSTRUCTION_SIZE 	      3
#define DESTINATION_INSTRUCTION_SIZE  3
#define COMP_INSTRUCTION_SIZE         6

#define SEMI_COLON 		     ';'
#define EQUAL_SIGN 		     '='

#define a_instruction(s)     s[0] == '@'


enum COMP
{
	COMP_ZERO 			=  0b00101010,
	COMP_ONE  			=  0b00111111,
	COMP_MINUS_ONE 			=  0b00111010,
	COMP_D 				=  0b00001100,
	COMP_A_OR_M 			=  0b00110000,
	COMP_NOT_D 			=  0b00001101,
	COMP_NOT_A_OR_NOT_M 		=  0b00110001,
	COMP_MINUS_D 			=  0b00001111,
	COMP_MINUS_A_OR_MINUS_M 	=  0b00110011,
	COMP_D_PLUS_ONE 		=  0b00011111,
	COMP_A_PLUS_ONE_OR_M_PLUS_ONE   =  0b00110111,
	COMP_D_MINUS_ONE 		=  0b00001110,
	COMP_A_MINUS_ONE_OR_M_MINUS_ONE =  0b00110010,
	COMP_D_PLUS_A_OR_D_PLUS_M 	=  0b00000010,
	COMP_D_MINUS_A_OR_D_MINUS_M 	=  0b00010011,
	COMP_A_MINUS_D_OR_M_MINUS_D     =  0b00000111,
	COMP_D_AND_A_OR_D_AND_M 	=  0b00000000,
	COMP_D_OR_A_OR_D_OR_M 		=  0b00010101
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
	MD 	= 0x03,
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

typedef struct SYMBOL_TABLE SYMBOL_TABLE;

struct SYMBOL_TABLE
{
	unsigned int 	size;
	unsigned int 	capacity;
	unsigned int 	symbol_starting_index; 
	
	       char **  symbols;
	unsigned int *  values;
	
};


SYMBOL_TABLE * init_symbol_table ();

unsigned int   add_symbol  	 (SYMBOL_TABLE *st, char *symbol, int line_number);
unsigned int * symbol_value	 (SYMBOL_TABLE *st, char *symbol);

char * 	       to_binary_string  (uint16_t bit_instruction);

uint16_t       translate	 (char *instruction, SYMBOL_TABLE *st);

uint16_t       comp_bits	 (char *comp_string);
uint16_t       destination_bits  (char *destination_string);
uint16_t       jump_bits         (char *jump_instruction);

char * 	       remove_whitespace (char *s) ;
bool 	       is_label		 (char *s);
bool 	       is_number	 (char *s);
bool 	       is_register	 (char *s);











#endif