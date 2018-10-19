#include <stdio.h>
#include <stdint.h>

void collect_data()
{

}

// checks if it's the expected char
int check_char(char current_char, char expected_char, int8_t next_state, int* error_state)
{
	int8_t current_state;

	if (current_char == expected_char) {
		current_state = next_state;
	}
	else {
		error_state = current_state;
		current_state = 17;
	}

	return current_state;
}

/*
- returns 0 if OK
- returns -1 if unknown error
- returns x, where x is the state number where the error occured
*/
int verify_response(char* _filename)
{
	FILE* f;
	char current_char;
	int8_t current_state = 0, error_state = -1;

	f = fopen(_filename, "rb");
	current_char = fgetc(f);

	while (current_char != EOF) {
		switch (current_state) {
		case 0:
			// expecting '<CR>'
			current_state = check_char(current_char, 0xD, 1, &error_state);		// TODO: is it necessary to check for error?
			break;
		case 1: 
			// expecting '<LF>'
			current_state = check_char(current_char, 0xA, 2, &error_state);
			break;
		case 2:
			// expecting '+'
			current_state = check_char(current_char, '+', 3, &error_state);
			break;
		case 3:
			// expecting any char, but '<CR>' or '<LF>'
			if (current_char == 0xD || current_char == 0xA) {		// TODO: is it necessary to check for '<LF>'?
				error_state = current_state;
				current_state = 17;
			}
			else {
				current_state = 4;
			}
			break;
		case 4:
			// expecting '<CR>' or any other char
			if (current_char == 0xD) {
				current_state = 5;
			}
			else if (current_char == 0xA) {								// TODO: is it necessary to check for '<LF>'?
				error_state = current_state;
				current_state = 17;
			}
			else {
				current_state = 4;
			}
			break;
		case 5:
			// expecting '<LF>'
			current_state = check_char(current_char, 0xA, 6, &error_state);
			break;
		case 6:
			// expecting '<CR>'
			current_state = check_char(current_char, 0xD, 7, &error_state);
			break;
		case 7:
			// expecting '<LF>'
			current_state = check_char(current_char, 0xA, 8, &error_state);
			break;
		case 8:
			// expecting 'O' or 'E'							// TODO: does it matter if it's caps?
			if (current_char == 'O') {
				current_state = 9;
			}
			else if (current_char == 'E') {
				current_state = 13;
			}
			else {
				error_state = current_state;
				current_state = 17;
			}
			break;
		case 9:
			// expecting 'K'
			current_state = check_char(current_char, 'K', 10, &error_state);
			break;
		case 10:
			break;
		case 11:
			break;
		case 12:
			break;
		case 13:
			break;
		case 14:
			break;
		case 15:
			break;
		case 16:
			break;
		case 17:
			return 0;
			break;
		default:
			printf("\nSomething went wrong!\n");
			return -1;
			break;
		}

		current_char = fgetc(f);
	}

	fclose(f);

	if (current_state == 12) {
		return 1;
	}
	else {
		return 0;
	}
}

int main()
{


	return 0;
}
