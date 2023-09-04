#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_INSTRUCTION_LENGTH 255
#define LEN_OPCODE 4
#define MAX_REGISTER_COUNT 8
#define MAX_LABEL_LEN 20
#define MAX_SYMBOL 255
#define NUM_PSEUDO_OPS 3
#define NUM_OPCODE 31

/* Symbol table containing address and string of label in assembly code*/
typedef struct{
	long int address;
	char label[MAX_LABEL_LEN+1];
} table_entry;

//used for determining label addresses
long int starting_address = 0;
long int current_address = 0;
int current_table_index = 0;

//symbol table containing all labels in the program
table_entry symbol_table[MAX_SYMBOL];

//enumerations related to the parsing, pseudo-ops, and type of registers used in the instruction processed, respectively
enum status{
	DONE, INVALID_LABEL, INVALID_OPCODE, INVALID_OPERAND, INVALID_PSEUDO_OP, EMPTY_FILE, TRUE, FALSE
};
enum pseudo_op{
	ORIG, FILL, END
};
enum bool_registers{
	DESTINATION_REGISTER, SOURCE_REGISTER, BASE_REGISTER
};

//various structs, mostly related to holding the name and machine code of the registers processed from instructions

typedef struct{
	char register_name[3];
	__int16_t machine_code;
} destination_register;

typedef struct{
	char register_name[3];
	__int16_t machine_code;
} source_register1;

typedef struct{
	char register_name[3];
	__int16_t machine_code;
} source_register2;

typedef struct{
	char register_name[3];
	__int16_t machine_code;
} base_register;

//arrays containing the pre-defined register structs
//TODO: Is there an easier way to do this without hard coding?
const destination_register arr_destination_register[] = {
	{"r0",0b0000000000000000},{"r1",0b0000001000000000},{"r2",0b0000010000000000},{"r3",0b0000011000000000},
	{"r4",0b0000100000000000},{"r5",0b0000101000000000},{"r6",0b0000110000000000},{"r7",0b0000111000000000}
};

const source_register1 arr_source_register1[] = {	
	{"r0",0b0000000000000000},{"r1",0b0000000001000000},{"r2",0b0000000010000000},{"r3",0b0000000011000000},
	{"r4",0b0000000100000000},{"r5",0b0000000101000000},{"r6",0b0000000110000000},{"r7",0b0000000111000000}
};

const source_register2 arr_source_register2[] = {	
	{"r0",0b0000000000000000},{"r1",0b0000000000000001},{"r2",0b0000000000000010},{"r3",0b0000000000000011},
	{"r4",0b0000000000000100},{"r5",0b0000000000000101},{"r6",0b0000000000000110},{"r7",0b0000000000000111}
};

const base_register arr_base_register[] = {	
	{"r0",0b0000000000000000},{"r1",0b0000000001000000},{"r2",0b0000000010000000},{"r3",0b0000000011000000},
	{"r4",0b0000000100000000},{"r5",0b0000000101000000},{"r6",0b0000000110000000},{"r7",0b0000000111000000},
};

typedef struct{	
	char opcode[6];
	int machine_code;
	int num_registers;
	int base_register;
	__int16_t num_bits_offset_immediate;
} opcode;

const opcode arr_opcode[] = {
	{"brn",0b0000100000000000, 0, 0, 9},{"brz",0b0000010000000000, 0, 0, 9},{"brp",0b0000001000000000, 0, 0, 9},{"brnz",0b0000110000000000, 0, 0, 9},
	{"brzp",0b0000011000000000, 0, 0, 9},{"brnp",0b0001010000000000, 0, 0, 9},{"br",0b0001110000000000, 0, 0, 9},{"brnzp",0b0000111000000000, 0, 0 ,9},
	{"add",0b0001000000000000, 3, 0, 0},{"add",0b0001000000100000, 2, 0, 5},		
	{"and",0b0101000000000000, 3, 0, 0},{"and",0b0101000000100000, 2, 0, 5},	
	{"jmp",0b1100000000000000, 1, 1, 0},	
	{"jsr",0b0100100000000000, 0, 0, 11},{"jsrr",0b0100000000000000, 1, 1, 0},	
	{"ldb",0b0010000000000000, 2, 1, 6},{"ldw",0b0110000000000000, 2, 1, 6},	
	{"lea",0b1110000000000000, 1, 0, 9},	
	{"not",0b1001000000111111, 2, 0, 0},	
	{"ret",0b1100000111000000, 0, 0, 0},	
	{"rti",0b1000000000000000, 0, 0, 0},	
	{"lshf",0b1101000000000000, 2, 0, 4},
	{"rshfl",0b1101000000010011, 2, 0, 4},
	{"rshfa",0b1101000000110000, 2, 0, 4},
	{"stb",0b0011000000110000, 2, 1, 6},	
	{"stw",0b0111000000110000, 2, 1, 6},
	{"trap",0b1111000000000000, 0, 0, 8},
	{"xor",0b1001000000000000, 3, 0, 0},
	{"xor",0b1001000000100000, 2, 0, 5},	
	{"nop",0b0000000000000000, 0, 0, 0},	
	{"halt",0b1111000000100101, 0, 0, 0}
};

//for string comparisons, too time consuming to refactor
const char* list_of_opcode[] = {
	"brn","brz","brp","brnz","brzp","brnp","br","brnzp","add","and","halt","jmp","jsr","jsrr","ldb","ldw","lea"
	,"nop","not","ret","lshf","rshfl","rshfa","rti","stb","stw","trap","xor"
};

const char* list_of_pseudo_op[3] = {
	".fill",".end",".orig"
};

int main(int argc, char **argv);

void parser_for_labels(FILE *input_assembly_file);

void convert_to_machine(FILE *input_assembly_file, FILE *object_code);

void write_to_file(FILE *output_file);

void add_label_to_table(char *label);

void set_start_address(char *instruction);

int which_opcode(char *token);

int check_alphanumeric(char *token);

char * prepend_zero(char *buffer, char *token);

int is_opcode(char *token);

int is_register_operand(char *token);

int is_immediate_operand(char *token);

int is_pseudo_op(char *token);

int main(int argc, char **argv){

	char *input_file = argv[1];
	char *output_file = argv[2];

	FILE *infile = fopen(input_file, "r");
	FILE *outfile = fopen(output_file, "w");

	if(!infile){
		printf("Cannot open input file: %s", input_file);
		exit(4);
	}
	
	if(!outfile){
		printf("Cannot open output file: %s", output_file);
		exit(4);
	}

	parser_for_labels(infile);

	rewind(infile);

	convert_to_machine(infile, outfile);

	fclose(infile);
	fclose(outfile);
	
	return 0;
}

void parser_for_labels(FILE *input_assembly_file){

}

void add_label_to_table(char *label){

}

void set_start_address(char *instruction){

}
int which_opcode(char *token){

}

int check_alphanumeric(char *token){

}

char * prepend_zero(char *buffer, char *token){

}

int is_opcode(char *token){

}

int is_register_operand(char *token){

}

int is_immediate_operand(char *token){

}

int is_pseudo_op(char *token){
	
}