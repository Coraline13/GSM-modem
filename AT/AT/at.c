#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "at.h"

#define CR 0xD
#define LF 0xA

#define STR_CNT 1000
#define STR_SIZE 200

/*typedef struct data {

};*/

// extracts the data and saves it in a file
void collect_data()
{
	FILE* f;

	f = fopen("data.out", "wb");

	fclose(f);
}

void print_data()
{

}

// returns true if the current state is either the succes state or the error state and false if not (and still waits for chars)
bool parse(int8_t* current_state, char current_char)
{
	switch (*current_state) {
	case INIT_STATE:
		// expecting '<CR>'
		*current_state = current_char == CR ? STATE_1 : ERROR_STATE;
		break;
	case STATE_1:
		// expecting '<LF>'
		*current_state = current_char == LF ? STATE_2 : ERROR_STATE;
		break;
	case STATE_2:
		// expecting '+' or 'O' or 'E'
		if (current_char == '+') {
			*current_state = STATE_3;
		}
		else if (current_char == 'O') {
			*current_state = STATE_9;
		}
		else if (current_char == 'E') {
			*current_state = STATE_13;
		}
		else {
			*current_state = ERROR_STATE;
		}
		break;
	case STATE_3:
		// expecting any char, but '<CR>' or '<LF>'
		if (current_char == CR || current_char == LF) {
			*current_state = ERROR_STATE;
		}
		else {
			*current_state = STATE_4;
		}
		break;
	case STATE_4:
		// expecting '<CR>' or any other char
		if (current_char == CR) {
			*current_state = STATE_5;
		}
		else if (current_char == LF) {
			*current_state = ERROR_STATE;
		}
		else {
			*current_state = STATE_4;
		}
		break;
	case STATE_5:
		// expecting '<LF>'
		*current_state = current_char == LF ? STATE_6 : ERROR_STATE;
		break;
	case STATE_6:
		// expecting '<CR>' or '+'
		if (current_char == CR) {
			*current_state = STATE_7;
		}
		else if (current_char == '+') {
			*current_state = STATE_3;
		}
		else {
			*current_state = ERROR_STATE;
		}
		break;
	case STATE_7:
		// expecting '<LF>'
		*current_state = current_char == LF ? STATE_8 : ERROR_STATE;
		break;
	case STATE_8:
		// expecting 'O' or 'E'
		if (current_char == 'O') {
			*current_state = STATE_9;
		}
		else if (current_char == 'E') {
			*current_state = STATE_13;
		}
		else {
			*current_state = ERROR_STATE;
		}
		break;
	case STATE_9:
		// expecting 'K'
		*current_state = current_char == 'K' ? STATE_10 : ERROR_STATE;
		break;
	case STATE_10:
		// expecting '<CR>'
		*current_state = current_char == CR ? STATE_11 : ERROR_STATE;
		break;
	case STATE_11:
		// expecting '<LF>'
		*current_state = current_char == LF ? SUCCES_STATE : ERROR_STATE;
		break;
	case STATE_13:
		// expecting 'R'
		*current_state = current_char == 'R' ? STATE_14 : ERROR_STATE;
		break;
	case STATE_14:
		// expecting 'R'
		*current_state = current_char == 'R' ? STATE_15 : ERROR_STATE;
		break;
	case STATE_15:
		// expecting 'O'
		*current_state = current_char == 'O' ? STATE_16 : ERROR_STATE;
		break;
	case STATE_16:
		// expecting 'R'
		*current_state = current_char == 'R' ? STATE_10 : ERROR_STATE;
		break;
	/*default:
		*current_state = ERROR_STATE;
		break;*/
	}

	return *current_state == SUCCES_STATE || *current_state == ERROR_STATE;
}
