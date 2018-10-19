#include <stdio.h>
#include "at.h"

int main(int argc, char **argv)
{
	// check if there is an argument to the program
	if (argc < 2) {
		printf("\nMissing argument!\n");
		return 1;
	}

	// test the given file
	if (verify_response(argv[1]) == 1) {
		printf("\nResponse OK!\n");
	}
	else {
		printf("\nResponse NOT OK!\n");
	}

	return 0;
}
