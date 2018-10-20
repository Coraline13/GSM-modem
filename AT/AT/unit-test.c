#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "at.h"

int main(int argc, char **argv)
{
	int8_t error_state = -1;

	// check if there is an argument to the program
	if (argc < 2) {
		printf("\nMissing argument!\n");
		return 1;
	}

	// test the given file
	if (verify_response(argv[1], &error_state) == 1) {
		printf("Response OK!\n\n");
	}
	else {
		printf("Response NOT OK!");

		if (error_state != -1) {
			printf(" It stops in state %" PRId8 ".", error_state);
		}

		printf("\n\n");
	}

	return 0;
}
