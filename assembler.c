/*
	Name 1: Paul Han
	UTEID 1: pjh2235
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define false 0
#define true 1
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
int table_length = 0;
int current_table_index = 0;

//symbol table containing all labels in the program
table_entry symbol_table[MAX_SYMBOL];

//enumerations related to the parsing, pseudo-ops, and type of registers used in the instruction processed, respectively
enum pseudo_op{
	ORIG, FILL, END
};
enum bool_registers{
	DESTINATION_REGISTER, SOURCE_REGISTER1, SOURCE_REGISTER2, BASE_REGISTER
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
	unsigned short machine_code;
	int num_registers;
	int base_register;
	__int16_t num_bits_offset_immediate;
} opcode;

const opcode arr_opcode[] = {
	{"brn",0b0000100000000000, 0, 0, 9},{"brz",0b0000010000000000, 0, 0, 9},{"brp",0b0000001000000000, 0, 0, 9},{"brnz",0b0000110000000000, 0, 0, 9},
	{"brzp",0b0000011000000000, 0, 0, 9},{"brnp",0b0000101000000000, 0, 0, 9},{"br",0b0000111000000000, 0, 0, 9},{"brnzp",0b0000111000000000, 0, 0 ,9},
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
	{"rshfl",0b1101000000010000, 2, 0, 4},
	{"rshfa",0b1101000000110000, 2, 0, 4},
	{"stb",0b0011000000000000, 2, 1, 6},	
	{"stw",0b0111000000000000, 2, 1, 6},
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
	".orig",".fill",".end"
};

int main(int argc, char **argv);

void parser_for_labels(FILE *input_assembly_file);

void convert_to_machine(FILE *input_assembly_file, FILE *object_code);

void print_bits(short machine_code);

void write_to_file(short machine_code, FILE *output_file);

void add_label_to_table(char *label);

void set_start_address(char *token);

int type_of_opcode(char *token);

int type_of_pseudo_op(char *token);

int value_of_register(char *token, int register_type);

int value_of_operand(char *token);

int check_alphanumeric(char *token);

char * prepend_zero(char *buffer, char *token);

int is_opcode(char *token);

int is_register_operand(char *token);

int is_immediate_operand(char *token);

int is_pseudo_op(char *token);

short convert_to_binary(int value, int num_bits);

int find_offset_of_label(char *label);

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
	char *instruction = (char *)malloc(sizeof(char)*MAX_INSTRUCTION_LENGTH);
	char *token = NULL;
	int end = false;
	while(fgets(instruction,MAX_INSTRUCTION_LENGTH,input_assembly_file)!=NULL || end == false){
		for(int i = 0; i < strlen(instruction); i++){
			instruction[i] = tolower(instruction[i]);
		}
		token = strtok(instruction, "\t\n ,");
		//reached end of assembly
		while(token!=NULL){
			//skip all comments
			if(strncmp(token,";",1)==0){
				break;
			}
			if(is_pseudo_op(token) == true){
				int pseudo_op = type_of_pseudo_op(token);
				if(pseudo_op == ORIG){
					token = strtok(NULL, "\t\n ,");
					set_start_address(token);
				}
				if(pseudo_op == END){
					end = true;
					break;
				}
			}
			if(is_pseudo_op(token) == false && is_opcode(token) == false && is_register_operand(token) == false && is_immediate_operand(token) == false){
				char *next_operand = strtok(NULL, "\t\n ,");
				if(next_operand == NULL || strncmp(next_operand,";",1)==0){
					break;
				}
				else
				{
					add_label_to_table(token);
					current_table_index++;
					table_length++;
					current_address++;
					break;
				}	
			}
			if(is_opcode(token) == true){
				current_address = current_address + 1;
			}
			token = strtok(NULL,"\t\n ,");
		}
	}
	free(instruction);
}

void convert_to_machine(FILE *input_assembly_file, FILE *object_code){	
	char *instruction = (char *)malloc(sizeof(char)*MAX_INSTRUCTION_LENGTH);
	char *token = NULL;
	int opcode_index = 0;
	char *opcode_string = (char *)malloc(sizeof(char)*MAX_INSTRUCTION_LENGTH);
	int num_registers = 0;
	char *register_operand[3];
	int operand = false;
	char *operand_string = (char *)malloc(MAX_INSTRUCTION_LENGTH);
	current_address = starting_address;
	int process_rest_of_instruction = true;
	int num_operands = 0;
	int end = false;
	while(fgets(instruction,MAX_INSTRUCTION_LENGTH,input_assembly_file)!=NULL || end == false){
		operand = false;
		num_registers = 0;
		num_operands = 0;
		for(int i = 0; i < strlen(instruction); i++){
			instruction[i] = tolower(instruction[i]);
		}
		token = strtok(instruction, "\t\n ,");
		//reached end of assembly
		while(token!=NULL){
			//skip all comments
			process_rest_of_instruction = true;
			if(strncmp(token,";",1)==0){
				break;
			}
			num_operands++;
			if(is_opcode(token)==true){
				opcode_index = type_of_opcode(token);
				opcode_string = strcpy(opcode_string, token);
				current_address = current_address + 1;
			}
			if(is_register_operand(token)==true){
				register_operand[num_registers] = token;
				num_registers++;
			}
			if(is_immediate_operand(token)==true){
				operand_string = strcpy(operand_string, token);
				operand = true;
			}
			if(is_pseudo_op(token) == true){
				int pseudo = type_of_pseudo_op(token);
				char *operand = strtok(NULL, "\t\n ,");
				if(pseudo == FILL){
					short value = value_of_operand(operand);
					write_to_file(value, object_code);	
					current_address = current_address + 1;
				}
				if(pseudo == ORIG){	
					short value = value_of_operand(operand);
					write_to_file(value, object_code);	
				}
				if(pseudo == END){
					num_operands = 0;
					end = true;
					break;
				}
			}
			if(is_opcode(token) == false && is_pseudo_op(token) == false && is_register_operand(token) == false && is_immediate_operand(token) == false){
				char *label = token;
				token = strtok(NULL,"\t\n ,");
				if(token == NULL || strncmp(token,";",1)==0){
					int offset = find_offset_of_label(label);
					sprintf(operand_string, "#%d", offset);
					operand = true;
					break;
				}else{
					if(is_pseudo_op(token)==true){
						int pseudo = type_of_pseudo_op(token);
						if(pseudo == FILL){
							char *operand = strtok(NULL, "\t\n ,");
							short value = value_of_operand(operand);
							write_to_file(value, object_code);	
							current_address = current_address + 1;
							num_operands = 0;
						}
					}
					if(is_opcode(token)==true){
						opcode_index = type_of_opcode(token);
						opcode_string = strcpy(opcode_string, token);
						process_rest_of_instruction = false;
					}
					
				}
			//TODO
			}	
			if(process_rest_of_instruction == true){
				token = strtok(NULL, "\t\n ,");
			}
		}
		unsigned short machine_code = 0;
		if(num_operands > 0){
			for(int i = 0; i < NUM_OPCODE; i++){
				if(strcmp(opcode_string,arr_opcode[i].opcode)==0 && num_registers == arr_opcode[i].num_registers){
					switch(num_registers){
						case 0:{
							if(operand == true){
								int value = value_of_operand(operand_string);
								machine_code = arr_opcode[i].machine_code;
								short value_operand = convert_to_binary(value, arr_opcode[i].num_bits_offset_immediate);
								machine_code |= value_operand;
							}else{
								machine_code = arr_opcode[i].machine_code;
							}	
							write_to_file(machine_code, object_code);
							break;
						}
						case 1:{
							//TODO: fix value_of_register and machine code conversions for negative immediate values
							if(arr_opcode[i].base_register == 0){
								int register_index = value_of_register(register_operand[0], DESTINATION_REGISTER);
								machine_code = arr_opcode[i].machine_code;
								machine_code |= arr_destination_register[register_index].machine_code;
								if(operand == true){
									int value = value_of_operand(operand_string);
									short value_operand = convert_to_binary(value, arr_opcode[i].num_bits_offset_immediate);
									machine_code |= value;
								}	
							}else{
								int register_index = value_of_register(register_operand[0], BASE_REGISTER);
								machine_code = arr_opcode[i].machine_code;
								machine_code |= arr_base_register[register_index].machine_code;
							}
							write_to_file(machine_code, object_code);
							break;
						}
						case 2:{
							int register_index = 0;
							register_index = value_of_register(register_operand[0], DESTINATION_REGISTER);
							machine_code = arr_opcode[i].machine_code;
							machine_code |= arr_destination_register[register_index].machine_code;
							if(arr_opcode[i].base_register == 1){
								register_index = value_of_register(register_operand[1], BASE_REGISTER);
								machine_code |= arr_base_register[register_index].machine_code;
							}
							else{
								register_index = value_of_register(register_operand[1], SOURCE_REGISTER1);
								machine_code |= arr_source_register1[register_index].machine_code;							
							}	
							int value = value_of_operand(operand_string);
							short value_operand = convert_to_binary(value, arr_opcode[i].num_bits_offset_immediate);
							machine_code |= value_operand;
							write_to_file(machine_code, object_code);
							break;
						}	
						case 3:{
							int source_register1_index = 0;
							int source_register2_index = 0;
							int destination_register_index = 0;
							source_register1_index = value_of_register(register_operand[1], SOURCE_REGISTER1);
							source_register2_index = value_of_register(register_operand[2], SOURCE_REGISTER2);
							destination_register_index = value_of_register(register_operand[0], DESTINATION_REGISTER);
							machine_code |= arr_source_register1[source_register1_index].machine_code;
							machine_code |= arr_source_register2[source_register2_index].machine_code;
							machine_code |= arr_destination_register[destination_register_index].machine_code;
							machine_code |= arr_opcode[i].machine_code;
							write_to_file(machine_code, object_code);
							break;
						}
					}
				}
			}	
		}
	}
	free(instruction);
	free(opcode_string);
	free(operand_string);
}
	

int find_offset_of_label(char *label){
	int found_label = false;
	int offset = 0;
	for(int i = 0; i < table_length; i++){
		if(strcmp(label, symbol_table[i].label)==0){
			offset = symbol_table[i].address - current_address;
		}
	}	
	current_table_index = 0;
	return offset;

}

short convert_to_binary(int value, int num_bits){
	short machine_code = 0;
	short bit_mask = 0b0000000000000001;
	int is_negative = false;
	if(value < 0){
		is_negative = true;
		value *= -1;
	}
	while(value!=0){	
		if (value%2==1)
		{
			machine_code |= bit_mask;
		}	
		bit_mask = bit_mask << 1;
		value /= 2;		
	}
	if(is_negative){
		bit_mask = 1;
		for(int i = 0; i < num_bits - 1; i++){
			bit_mask = bit_mask<<1;
		}	
		for(int i = 0; i < num_bits; i++){
			machine_code ^= bit_mask;
			bit_mask = bit_mask >> 1;
		}
		machine_code += 1;	
	}
	return machine_code;
}

void write_to_file(short machine_code, FILE *output_file){
	char *data = (char *)malloc(sizeof(char)*50);
	data[0] = '0';
	data[1] = 'x';
	sprintf(&data[2],"%04hX\n",machine_code);
	fprintf(output_file, "%s", data);
	free(data);
}

void add_label_to_table(char *label){
	symbol_table[current_table_index].address = current_address;
	strcpy(symbol_table[current_table_index].label, label);
}

void set_start_address(char *token){
	starting_address = value_of_operand(token);
	current_address = starting_address;
}

int check_alphanumeric(char *token){

}

char * prepend_zero(char *buffer, char *token){
	buffer[0] = '0';
	buffer++;
	buffer = strcpy(buffer,token);
	buffer--;
	return buffer;
}

int is_opcode(char *token){
	for(int i = 0; i < NUM_OPCODE; i++){
		if(strcmp(token,arr_opcode[i].opcode)==0){
			return true;
		}
	}
	return false;
}

int type_of_opcode(char *token){
	for(int i = 0; i < NUM_OPCODE; i++){
		if(strcmp(token,arr_opcode[i].opcode)==0){
			return i;
		}
	}
	//input should always be an actual opcode
	return false;
}

int is_register_operand(char *token){
	//R + register number + null terminator = 3 bytes
	char *register_operand = (char *)malloc(sizeof(char)*3);
	register_operand[0] = 'r';
	int is_register = false;
	// printf("token register: %s\n", token);
	for(int i = 0; i < 8; i++){
		sprintf(&register_operand[1], "%d", i);
		if(strcmp(token, register_operand)==0){		
			// printf("matching register: %s\n", register_operand);
			is_register = true;
		}
	}
	free(register_operand);
	return is_register;
}

int value_of_register(char *token, int register_type){
	int index = strtol(&token[1],NULL,0);
	return index;
}

int is_immediate_operand(char *token){
	if(strncmp(token,"#",1)==0 || strncmp(token,"x",1)==0){
		return true;
	}
	return false;
}

int value_of_operand(char *token){
	char *buffer = (char *)malloc((strlen(token)+1)*sizeof(char));
	short value = 0;
	if(strncmp(token,"#",1)==0){
		value = strtol(&token[1],NULL,0);
	}
	if(strncmp(token,"x",1)==0){
		buffer = prepend_zero(buffer, token);
		value = strtol(buffer, NULL, 16);
	}
	free(buffer);
	// printf("immediate value: %d\n", value);
	return value;
}

int is_pseudo_op(char *token){
	for(int i = 0; i < NUM_PSEUDO_OPS; i++){
		if(strcmp(token,list_of_pseudo_op[i])==0){
			return true;
		}
	}	
	return false;
}

int type_of_pseudo_op(char *token){
	int index = 0;
	for(int i = 0; i < NUM_PSEUDO_OPS; i++){
		if(strcmp(token, list_of_pseudo_op[i])==0){
			index = i;
		}
	}
	//for readability purposes
	for(int i = ORIG; i <= END; i++){
		if(index == i){	
			return i;
		}
	}
}