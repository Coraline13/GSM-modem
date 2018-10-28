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
	int i;
	printf("Line count: %d\n", data.line_count);
	for(i = 0; i < data.line_count; i++) {
		printf("%s\n", data.data[i]);
	}
	printf("%s\n", data.ok ? "OK" : "ERROR");
}

// returns true if the current state is either the succes state or the error state and false if not (and still waits for chars)
bool parse(int8_t* current_state, char current_char)
{
	switch (*current_state) {
	case INIT_STATE:
		// expecting '<CR>'
		*current_state = current_char == CR ? STATE_1 : ERROR_STATE;
		// reset data structure
		memset(&data, 0x00, sizeof(data));
		break;
	case STATE_1:
		// expecting '<LF>'
		*current_state = current_char == LF ? STATE_2 : ERROR_STATE;
		break;
	case STATE_2:
		// expecting '+' or 'O' or 'E'
		if (current_char == '+') {
			*current_state = STATE_3;
			append_char(data.data[data.line_count], current_char); //adds '+' to the current string in data
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
			append_char(data.data[data.line_count], current_char); // appends current_char to the current line in data
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
			append_char(data.data[data.line_count], current_char); // appends current_char to the current line in data unless current_char is CR or LF
		}
		break;
	case STATE_5:
		// expecting '<LF>'
		*current_state = current_char == LF ? STATE_6 : ERROR_STATE;
		break;
	case STATE_6:
		// if we got here, it means we have parsed a full line of output, so we prepare the next one
		append_char(data.data[data.line_count], 0); // add \0 to the end of the current line
		data.line_count++;
		// expecting '<CR>' or '+'
		if (current_char == CR) {
			*current_state = STATE_7;
		}
		else if (current_char == '+') {
			*current_state = STATE_3;
			append_char(data.data[data.line_count], current_char); // append '+' for the next line of output
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
		// if we got here, than the response is OK
		data.ok = true;
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
		// if we got here, than the response had an error
		data.ok = false;
		break;
	/*default:
		*current_state = ERROR_STATE;
		break;*/
	}

	return *current_state == SUCCES_STATE || *current_state == ERROR_STATE;
}
