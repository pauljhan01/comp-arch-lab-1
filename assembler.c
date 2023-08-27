#include "assembler.h"

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


	fclose(infile);
	fclose(outfile);
	
	return 0;
}
