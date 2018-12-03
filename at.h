/*
	Team:
	Bartels Mathieu
	Bocereg Alexandra
*/
#include<stdint.h>
#include<stdarg.h>

#define AT_COMMAND_MAX_LINES 100
#define AT_COMMAND_MAX_LINE_SIZE 128

typedef struct
{
	uint8_t ok;
	uint8_t data[AT_COMMAND_MAX_LINES][AT_COMMAND_MAX_LINE_SIZE + 1];
	uint32_t line_count;
}AT_COMMAND_DATA;
extern AT_COMMAND_DATA info;
typedef enum
{
	STATE_MACHINE_NOT_READY,
	STATE_MACHINE_READY_OK,
	STATE_MACHINE_READY_WITH_ERROR
}STATE_MACHINE_RETURN_VALUE;

STATE_MACHINE_RETURN_VALUE at_command_parse(uint8_t current_character);

void print_AT_commands();
