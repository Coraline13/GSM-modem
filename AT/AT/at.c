#include <stdio.h>
#include <stdint.h>

void collect_data()
{

}

int verify_response(char* filename)
{
	FILE* f;
	char current_char;
	uint8_t current_state = 0;

	f = fopen(filename, "rb");
	current_char = fgetc(f);

	while (current_char != EOF) {
		switch (current_state) {
		case 0:
			if (current_char == 0xD) {
				current_state = 1;
			}
			break;
		case 1: 
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;
		case 9:
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
