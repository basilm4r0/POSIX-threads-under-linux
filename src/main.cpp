#include "local.h"

int main(int argc, char *argv[]) {
	read_constants();
}

int read_constants(char *filename) {
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		perror("fopen");
		return 1;
	}

	int num_helpers, num_spies;
	if (fscanf(file, "NUM_HELPERS=%d\n", &num_helpers) != 1) {
		printf("Error reading NUM_HELPERS from file\n");
		fclose(file);
		return 1;
	}

	if (fscanf(file, "NUM_SPIES=%d\n", &num_spies) != 1) {
		printf("Error reading NUM_SPIES from file\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "WORD_LENGTH=%d\n", &WORD_LENGTH) != 1) {
		printf("Error reading WORD_LENGTH from file\n");
		fclose(file);
		return 1;
	}
	if (fscanf(file, "NUM_LINES=%d\n", &NUM_LINES) != 1) {
		printf("Error reading NUM_LINES from file\n");
		fclose(file);
		return 1;
	}
	fclose(file);

	// Assign the values to the global variables
	NUM_HELPERS = num_helpers;
	NUM_SPIES = num_spies;

	// Rest of your program logic goes here

	return 0;
}
