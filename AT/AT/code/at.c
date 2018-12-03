#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "at.h"

#define CR 0xD
#define LF 0xA

AT_DATA data;

static void append_char(char str[], char c) {
	size_t l = strlen(str);
	str[l] = c;
}

void print_data()
{
	unsigned int i;

	printf("Line count: %d\n", data.line_count + 1);

	for(i = 0; i < data.line_count; i++) {
		printf("%s\n", data.data[i]);
	}

	printf("%s\n", data.ok ? "OK" : "ERROR");
}

// returns true if the current state is either the success state or the error state and false if not (and still waits for chars)
uint8_t parse(char current_char, uint8_t command_flag)
{
	static uint8_t state = 0;

	if (state == SUCCESS_STATE || state == ERROR_STATE) {
		state = 0;
	}

	switch (state) {
	case INIT_STATE:
		// expecting '<CR>'
		state = current_char == CR ? STATE_1 : ERROR_STATE;

		// reset data structure
		memset(&data, 0x00, sizeof(data));
		break;
	case STATE_1:
		// expecting '<LF>'
		state = current_char == LF ? STATE_2 : ERROR_STATE;
		break;
	case STATE_2:
		// expecting '+' or 'O' or 'E' or a special command_flag
		if (current_char == '+' || command_flag == AT_GMI || command_flag == AT_GMR || command_flag == AT_GSN) {
			state = STATE_3;

			// adding '+' to the current string in data
			append_char(data.data[data.line_count], current_char);
		}
		else if (current_char == 'O') {
			state = STATE_9;
		}
		else if (current_char == 'E') {
			state = STATE_13;
		}
		else {
			state = ERROR_STATE;
		}
		break;
	case STATE_3:
		// expecting any char but '<CR>' or '<LF>'
		if (current_char == CR || current_char == LF) {
			state = ERROR_STATE;
		}
		else {
			state = STATE_4;

			// appending the current char to the current line in data
			append_char(data.data[data.line_count], current_char);
		}
		break;
	case STATE_4:
		// expecting '<CR>' or any other char
		if (current_char == CR) {
			state = STATE_5;
		}
		else if (current_char == LF) {
			state = ERROR_STATE;
		}
		else {
			state = STATE_4;

			// appending current_char to the current line in data unless current_char is CR or LF
			append_char(data.data[data.line_count], current_char);
		}
		break;
	case STATE_5:
		// expecting '<LF>'
		state = current_char == LF ? STATE_6 : ERROR_STATE;
		break;
	case STATE_6:
		// if we got here, it means we have parsed a full line of output, so we prepare the next one
		// adding \0 to the end of the current line
		append_char(data.data[data.line_count], 0);
		data.line_count++;

		// expecting '<CR>' or '+'
		if (current_char == CR) {
			state = STATE_7;
		}
		else if (current_char == '+') {
			state = STATE_3;

			// appending '+' for the next line of the output
			append_char(data.data[data.line_count], current_char);
		}
		else {
			state = ERROR_STATE;
		}
		break;
	case STATE_7:
		// expecting '<LF>'
		state = current_char == LF ? STATE_8 : ERROR_STATE;
		break;
	case STATE_8:
		// expecting 'O' or 'E'
		if (current_char == 'O') {
			state = STATE_9;
		}
		else if (current_char == 'E') {
			state = STATE_13;
		}
		else {
			state = ERROR_STATE;
		}
		break;
	case STATE_9:
		// expecting 'K'
		state = current_char == 'K' ? STATE_10 : ERROR_STATE;

		// if we got here, then the response is OK
		data.ok = true;
		break;
	case STATE_10:
		// expecting '<CR>'
		state = current_char == CR ? STATE_11 : ERROR_STATE;
		break;
	case STATE_11:
		// expecting '<LF>'
		state = current_char == LF ? SUCCESS_STATE : ERROR_STATE;
		break;
	case STATE_13:
		// expecting 'R'
		state = current_char == 'R' ? STATE_14 : ERROR_STATE;
		break;
	case STATE_14:
		// expecting 'R'
		state = current_char == 'R' ? STATE_15 : ERROR_STATE;
		break;
	case STATE_15:
		// expecting 'O'
		state = current_char == 'O' ? STATE_16 : ERROR_STATE;
		break;
	case STATE_16:
		// expecting 'R'
		state = current_char == 'R' ? STATE_10 : ERROR_STATE;

		// if we got here, then the response had an ERROR
		data.ok = false;
		break;
	default:
		state = ERROR_STATE;
		break;
	}

	return state;
}