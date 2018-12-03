/*
	Team:
	Bartels Mathieu
	Bocereg Alexandra
*/
#include "at.h"
#include<stdio.h>
AT_COMMAND_DATA info;

STATE_MACHINE_RETURN_VALUE at_command_parse(uint8_t current_character)
{
	static uint32_t state = 0;
	static uint32_t col_index = 0;
	switch (state)
	{
		case 0:
		{
			if (current_character == 0x0D) //CR
			{
				state = 1;
			}
			else {
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 1:
		{
			if (current_character == 0x0A) //LF
			{
				state = 2;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 2:
		{
			if (current_character == 'O')
			{
				state = 3;
			}
			else if (current_character == 'E')
			{
				state = 7;
			}
			else if (current_character == '+')
			{
				state = 14;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 3:
		{
			if (current_character == 'K')
			{
				state = 4;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 4:
		{
			if (current_character == 0x0D)
			{
				state = 5;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 5:
		{
			if (current_character == 0x0A)
			{
				info.ok = 1;
				state = 0;
				return STATE_MACHINE_READY_OK;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 7:
		{
			if (current_character == 'R')
			{
				state = 8;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 8:
		{
			if (current_character == 'R')
			{
				state = 9;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 9:
		{
			if (current_character == 'O')
			{
				state = 10;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 10:
		{
			if (current_character == 'R')
			{
				state = 11;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 11:
		{
			if (current_character == 0x0D)
			{
				state = 12;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 12:
		{
			if (current_character == 0x0A)
			{
				info.ok = 0;
				state = 0;
				//return STATE_MACHINE_READY_OK;
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}

		case 14:
		{
			if (current_character >= 0x20 && current_character <= 0x7E)
			{
				state = 14;
				if (info.line_count < AT_COMMAND_MAX_LINES) {
					if (col_index < AT_COMMAND_MAX_LINE_SIZE) {
						info.data[info.line_count][col_index] = current_character;
						col_index++;
					}
				}
			}
			else if (current_character == 0x0D)
			{
				if (info.line_count < AT_COMMAND_MAX_LINES) {
					info.data[info.line_count][col_index] = '\0';
					col_index = 0;
					info.line_count++;
				}
				state = 15;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 15:
		{
			if (current_character == 0x0A)
			{
				state = 16;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 16:
		{
			if (current_character == 0x0D)
			{
				state = 17;
			}
			else if ((current_character == '+') || (special_command))
			{
				state = 14;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}

		case 17:
		{
			if (current_character == 0x0A)
			{
				state = 18;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
		case 18:
		{
			if (current_character == 'O')
			{
				state = 3;
			}
			else if (current_character == 'E')
			{
				state = 7;
			}
			else
			{
				return STATE_MACHINE_READY_WITH_ERROR;
			}
			break;
		}
	}
	return STATE_MACHINE_NOT_READY;
}

void print_AT_commands() {
	if (info.line_count == 0) {
		printf("No AT commands in response.\n");
	}
	else {
		uint32_t i = 0;
		for (i = 0; i < info.line_count; i++) {
			printf("%s\n", info.data[i]);
		}
		printf("\nEnd of AT commands.\n");
	}
}
