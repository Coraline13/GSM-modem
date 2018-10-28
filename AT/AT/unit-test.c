#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include "at.h"

// receives a file and returns true if it is ok and false if it is not
bool verify_response(char* filename, int8_t* error_state)
{
	FILE* f;
	char current_char;
	int8_t current_state = INIT_STATE;
	int8_t previous_state = current_state;
	bool response = false;

	f = fopen(filename, "rb");

	if (!f) {
		printf("File couldn't be open.\n");
		return false;
	}

	current_char = fgetc(f);

	while (current_char != EOF && !response) {
		previous_state = current_state;
		response = parse(&current_state, current_char);
		current_char = fgetc(f);
	}

	fclose(f);

	if (current_state == SUCCESS_STATE) {
		return true;
	}
	else {
		*error_state = previous_state;
		return false;
	}
}

int main(int argc, char **argv)
{
	int8_t error_state = -1;

	// check if there is an argument to the program
	if (argc < 2) {
		printf("\nMissing argument!\n");
		return 1;
	}

	// test the given file
	if (verify_response(argv[1], &error_state) == true) {
		printf("Response OK!\n\n");
		print_data();
	}
	else {
		printf("Response NOT OK!");

		if (error_state != -1) {
			printf(" It stops in state %" PRId8 ".\n", error_state);
		}
	}

	printf("\n");

	return 0;
}
