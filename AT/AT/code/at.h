#ifndef AT_H
#define AT_H

#define INIT_STATE 0
#define STATE_1 1
#define STATE_2 2
#define STATE_3 3
#define STATE_4 4
#define STATE_5 5
#define STATE_6 6
#define STATE_7 7
#define STATE_8 8
#define STATE_9 9
#define STATE_10 10
#define STATE_11 11
#define SUCCESS_STATE 12
#define STATE_13 13
#define STATE_14 14
#define STATE_15 15
#define STATE_16 16
#define ERROR_STATE 17

#define AT_CSQ 0
#define AT_CREG 1
#define AT_COPS 2
#define AT_GSN 3
#define AT_GMI 4
#define AT_GMR 5
#define AT_CMGL 6
#define AT_CMGS 7

#define STR_CNT 100
#define STR_SIZE 300

#define CR 0xD
#define LF 0xA
#define SUB 0x1A

typedef struct {
  char data[STR_CNT][STR_SIZE + 1];
  uint16_t line_count;
  bool ok;
} AT_DATA;

extern AT_DATA data;

void print_data();

uint8_t parse(char current_char, uint8_t command_flag);

#endif
