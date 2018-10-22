#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define SUCCES_STATE 12
#define ERROR_STATE 17

/*typedef struct data {

};*/

// extracts the data and saves it in a file
void collect_data()
{
	FILE* f;

	f = fopen("data.out", "wb");

}

// checks if it's the expected char and returns the next state
int check_char(char current_char, char expected_char, int8_t current_state, int8_t next_state, int8_t* error_state)
{
	if (current_char == expected_char) {
		current_state = next_state;
	}
	else {
		*error_state = current_state;
		current_state = ERROR_STATE;
	}

	return current_state;
}

/*
- returns 1 if OK
- returns 0 if error (and error_state will be x, where x is the state number where the error occured)
*/
bool verify_response(char* filename, int8_t* error_state)
{
	FILE* f;
	char current_char;
	int8_t current_state = 0;

	f = fopen(filename, "rb");
	current_char = fgetc(f);

	while (current_char != EOF) {
		switch (current_state) {
		case 0:
			// expecting '<CR>'
			current_state = check_char(current_char, 0xD, 0, 1, error_state);		// TODO: is it necessary to check for error?
			break;
		case 1: 
			// expecting '<LF>'
			current_state = check_char(current_char, 0xA, 1, 2, error_state);
			break;
		case 2:
			// expecting '+' or 'O' or 'E'							// TODO: does it matter if it's caps?
			if (current_char == '+') {
				current_state = 3;
			}
			else if (current_char == 'O') {
				current_state = 9;
			}
			else if (current_char == 'E') {
				current_state = 13;
			}
			else {
				*error_state = current_state;
				current_state = ERROR_STATE;
			}
			break;
		case 3:
			// expecting any char, but '<CR>' or '<LF>'
			if (current_char == 0xD || current_char == 0xA) {		// TODO: is it necessary to check for '<LF>'?
				*error_state = current_state;
				current_state = ERROR_STATE;
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
				*error_state = current_state;
				current_state = ERROR_STATE;
			}
			else {
				current_state = 4;
			}
			break;
		case 5:
			// expecting '<LF>'
			current_state = check_char(current_char, 0xA, 5, 6, error_state);
			break;
		case 6:
			// expecting '<CR>' or '+'
			if (current_char == 0xD) {
				current_state = 7;
			}
			else if (current_char == '+') {
				current_state = 3;
			}
			else {
				*error_state = current_state;
				current_state = ERROR_STATE;
			}
			break;
		case 7:
			// expecting '<LF>'
			current_state = check_char(current_char, 0xA, 7, 8, error_state);
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
				*error_state = current_state;
				current_state = ERROR_STATE;
			}
			break;
		case 9:
			// expecting 'K'
			current_state = check_char(current_char, 'K', 9, 10, error_state);
			break;
		case 10:
			// expecting '<CR>'
			current_state = check_char(current_char, 0xD, 10, 11, error_state);
			break;
		case 11:
			// expecting '<LF>'
			current_state = check_char(current_char, 0xA, 11, SUCCES_STATE, error_state);
			break;
		case SUCCES_STATE:
			return true;
			break;
		case 13:
			current_state = check_char(current_char, 'R', 13, 14, error_state);
			break;
		case 14:
			current_state = check_char(current_char, 'R', 14, 15, error_state);
			break;
		case 15:
			current_state = check_char(current_char, 'O', 15, 16, error_state);
			break;
		case 16:
			current_state = check_char(current_char, 'R', 16, 10, error_state);
			break;
		case ERROR_STATE:
			return false;
			break;
		}

		current_char = fgetc(f);
	}

	fclose(f);

	if (current_state == SUCCES_STATE) {
		return true;
	}
	else {
		return false;
	}
}
