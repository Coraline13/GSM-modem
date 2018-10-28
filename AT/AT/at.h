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

#define STR_CNT 1000
#define STR_SIZE 200

typedef struct {
  char data[STR_CNT][STR_SIZE + 1];
  uint32_t line_count;
  bool ok;
} AT_DATA;

extern AT_DATA data;

void print_data();

bool parse(int8_t* current_state, char current_char);

#endif
