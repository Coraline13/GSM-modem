#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include "LPC177x_8x.h"
#include "system_LPC177x_8x.h"
#include <retarget.h>

#include <DRV\drv_sdram.h>
#include <DRV\drv_lcd.h>
#include <DRV\drv_uart.h>
#include <DRV\drv_touchscreen.h>
#include <DRV\drv_led.h>
#include <utils\timer_software.h>
#include <utils\timer_software_init.h>

#include "at.h"
#define SIM_STORAGE_SIZE 10

uint8_t message_count = SIM_STORAGE_SIZE;

typedef struct {
	char nr[13];
	char text[161];
	char timestamp[21];
} SMS;

SMS messages[SIM_STORAGE_SIZE];

char at_command_at[] = "AT\r\n";
char at_command_csq[] = "AT+CSQ\r\n";
char at_command_creg[] = "AT+CREG?\r\n";
char at_command_cops[] = "AT+COPS?\r\n";
char at_command_gsn[] = "AT+GSN\r\n";
char at_command_gmi[] = "AT+GMI\r\n";
char at_command_gmr[] = "AT+GMR\r\n";
char at_command_extended_creg_1[] = "AT+CREG=1\r\n";
char at_command_extended_creg_2[] = "AT+CREG=2\r\n";

char at_command_cmgl[] = "AT+CMGL=\"ALL\"\r\n";

LCD_PIXEL white = {255, 255, 255};
LCD_PIXEL primary = {95, 75, 139};
LCD_PIXEL black = {0, 0, 0};

timer_software_handler_t handler_main;
timer_software_handler_t handler_get_response;
timer_software_handler_t handler_buttons;

void timer_callback_1(timer_software_handler_t h)
{
}

void drawButtons(uint8_t, uint8_t, uint16_t, uint16_t, LCD_PIXEL);
void displaySMS(uint8_t);

void TouchScreenCallBack(TouchResult* touchData)
{
	static uint8_t sms_id = 0;
	int i, j;
	
	//printf("touched X=%3d Y=%3d\n", touchData->X, touchData->Y);	
	handler_buttons = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(handler_buttons, MODE_1, 200, 1);
	
	TIMER_SOFTWARE_reset_timer(handler_buttons);
	TIMER_SOFTWARE_Wait(200);
	
	if(touchData->Y >= 200) {
		if(touchData->X >= 0 && touchData->X <= LCD_WIDTH/4) {
			printf("<Prev button was pressed\n");
			sms_id = (sms_id == 0) ? message_count - 1 : sms_id - 1;
			displaySMS(sms_id);
		} else if(touchData->X >= LCD_WIDTH/4 && touchData->X <= LCD_WIDTH/2) {
			printf("Delete button was pressed\n");
			for(i = 0; i < LCD_WIDTH; i++){
				for(j = 41; j < 200; j++) {
					DRV_LCD_PutPixel(j, i, white.red, white.green, white.blue);
				}
			}
			for(i = sms_id; i < message_count - 1; i++) {
				messages[i] = messages[i+1];
			}
			if(sms_id == message_count-1) sms_id = 0;
			message_count--;
			DRV_LCD_Puts("MESSAGE DELETED", 30, 65, black, white, 1);
			TIMER_SOFTWARE_Wait(300);
			displaySMS(sms_id);
		} else if(touchData->X >= LCD_WIDTH/2 && touchData->X <= 3*LCD_WIDTH/4) {
			printf("Send button was pressed\n");
			for(i = 0; i < LCD_WIDTH; i++){
				for(j = 41; j < 200; j++) {
					DRV_LCD_PutPixel(j, i, white.red, white.green, white.blue);
				}
			}
			DRV_LCD_Puts("MESSAGE SENT", 30, 65, black, white, 1);
		} else {
			printf("Next> button was pressed\n");
			sms_id = (sms_id == message_count - 1) ? 0 : sms_id + 1;
			displaySMS(sms_id);
		}
	}
}

void addPlaceholderText(SMS *messages)
{	
	strcpy(messages[0].nr, "0765980589");
	strcpy(messages[0].text, "Unde esti?");
	strcpy(messages[0].timestamp, "0. 11:54");
	
	strcpy(messages[1].nr, "0770715451");
	strcpy(messages[1].text, "Pe drum");
	strcpy(messages[1].timestamp, "1. 11:59");
	
	strcpy(messages[2].nr, "0765980589");
	strcpy(messages[2].text, "Spre?");
	strcpy(messages[2].timestamp, "2. 12:04");
	
	strcpy(messages[3].nr, "0770715451");
	strcpy(messages[3].text, "Spre camin. Tu unde esti?");
	strcpy(messages[3].timestamp, "3. 12:11");
	
	strcpy(messages[4].nr, "0765980589");
	strcpy(messages[4].text, "Te astept... De 34 de minute si 15 secunde...");
	strcpy(messages[4].timestamp, "4. 12:12");
	
	strcpy(messages[5].nr, "0770715451");
	strcpy(messages[5].text, "...scuze? Ajung imediat.");
	strcpy(messages[5].timestamp, "5. 12:15");
	
	strcpy(messages[6].nr, "0765980589");
	strcpy(messages[6].text, "?_?");
	strcpy(messages[6].timestamp, "6. 12:16");
	
	strcpy(messages[7].nr, "0770715451");
	strcpy(messages[7].text, "^.^");
	strcpy(messages[7].timestamp, "7. 12:18");

	strcpy(messages[8].nr, "0765980589");
	strcpy(messages[8].text, "Te astept la intrare.");
	strcpy(messages[8].timestamp, "8. 12:21");
	
	strcpy(messages[9].nr, "0770715451");
	strcpy(messages[9].text, "Acu ajung.");
	strcpy(messages[9].timestamp, "9. 12:24");
}

void displaySMS(uint8_t id) 
{
	int i, j;
	for(i = 0; i < LCD_WIDTH; i++){
		for(j = 41; j < 200; j++) {
			DRV_LCD_PutPixel(j, i, white.red, white.green, white.blue);
		}
	}
	DRV_LCD_Puts(messages[id].nr, 30, 65, black, white, 1);	
	for(i = 10; i < 300; i++) {
		DRV_LCD_PutPixel(85, i, black.red, black.green, black.blue);
	}
	DRV_LCD_Puts(messages[id].text, 20, 110, black, white, 0);	
	DRV_LCD_Puts(messages[id].timestamp, 20, 120, black, white, 0);	
}

void BoardInit()
{		
	int i, j;
	
	TIMER_SOFTWARE_init_system();
	
	DRV_SDRAM_Init();
	
	initRetargetDebugSystem();
	DRV_LCD_Init();
	DRV_LCD_ClrScr();
	DRV_LCD_PowerOn();	
	
	DRV_TOUCHSCREEN_Init();
	DRV_TOUCHSCREEN_SetTouchCallback(TouchScreenCallBack);
	DRV_LED_Init();
	
	printf("Hello\n");	
	
	DRV_LCD_TestFillColor(255, 255, 255);
	drawButtons(0, 200, LCD_WIDTH, LCD_HEIGHT, primary);
	
	for(i = 0; i <= 40; i++) {
		for(j = 0; j <= LCD_WIDTH; j++) {
			DRV_LCD_PutPixel(i, j, primary.red, primary.green, primary.blue);
		}
	}
	DRV_LCD_Puts("GSM modem signal: <acquiring data>", 20, 5, white, primary, 0);
	DRV_LCD_Puts("Network state: <acquiring data>", 20, 18, white, primary, 0);
	DRV_LCD_Puts("Network operator name: <acquiring data>", 20, 30, white, primary, 0);
	
	addPlaceholderText(messages);
	displaySMS(0);
}
              
void send_command(char *cmd) {
	DRV_UART_FlushRX(UART_3);
	DRV_UART_FlushTX(UART_3);
	DRV_UART_Write(UART_3, (uint8_t*) cmd, strlen(cmd)); 
}

void get_command_response(uint8_t command_flag) {
	uint8_t ch;
	uint8_t state = 0;
	handler_get_response = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(handler_get_response, MODE_1, 10000, 1);
	
	TIMER_SOFTWARE_reset_timer(handler_get_response);
	TIMER_SOFTWARE_start_timer(handler_get_response);
	
	while (!TIMER_SOFTWARE_interrupt_pending(handler_get_response) && (state != SUCCESS_STATE)){
		
		while (DRV_UART_BytesAvailable(UART_3) > 0 && (state != ERROR_STATE)) {
			DRV_UART_ReadByte(UART_3, &ch);
			state = parse(ch, command_flag);
		}
	}
}

void execute_command(char *cmd, uint8_t command_flag) {
	send_command(cmd);
	get_command_response(command_flag);
}

bool verify_response(AT_DATA *data) {	
	return data->ok;
}

uint32_t get_asu_from_response(AT_DATA *data){
	return strtol(&data->data[0][6], NULL, 10);
}

int32_t asu_to_dbmw(uint32_t asu) {
	return 2 * asu - 113;
}

// for AT_COPS
char* get_operator_name(AT_DATA *data) {
	char *op_name;
	op_name = strstr(&data->data[0][8], "\"");
	return op_name;
}

// for AT_GSN
char* get_imei(AT_DATA *data) {
	return data->data[0];
}

// for AT_GMI
char* get_manufacturer_identity(AT_DATA *data) {
	return data->data[0];
}

// for AT_GMR
char* get_software_version(AT_DATA *data) {
	return &data->data[0][10];
}

// for AT_CREG
char* get_network_state(AT_DATA *data) {
	uint8_t stat;
	stat = strtol(&data->data[0][9], NULL, 10);
	switch (stat) {
	case 0: return "Modem not searching for network";
	case 1: return "Modem is registered to home network";
	case 2: return "Modem searching for network";
	case 3: return "Modem registration into the network was denied";
	case 4: return "Unknown modem registration state";
	case 5: return "Modem is registered to roaming network";
	default: return "Unknown state";
	}
}


void drawButtons(uint8_t x, uint8_t y, uint16_t width, uint16_t height, LCD_PIXEL bg_color) {
	int i, j;
	for(i = y; i <= height; i++) {
		for(j = x; j <= width; j++) {
			DRV_LCD_PutPixel(i, j, bg_color.red, bg_color.green, bg_color.blue);
		}
	}
	DRV_LCD_Puts("<Prev", x+30, y+25, white, primary, 1);
	DRV_LCD_Puts("Delete", x+150, y+25, white, primary, 1);	
	DRV_LCD_Puts("Send", x+290, y+25, white, primary, 1);
	DRV_LCD_Puts("Next>", x+390, y+25, white, primary, 1);
}

int main(void)
{
	uint32_t rssi_value_asu;
	int32_t rssi_value_dbmw;
	char s[100];
	int i,j;

	BoardInit();
	
	DRV_UART_Configure(UART_3, UART_CHARACTER_LENGTH_8, 115200, UART_PARITY_NO_PARITY, 1, TRUE, 3);
	
	handler_main = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(handler_main, MODE_1, 1000, 1);
	// send AT command to tell modem to autobaud
	send_command(at_command_at);
	TIMER_SOFTWARE_Wait(1000);
	
	send_command(at_command_at);
	TIMER_SOFTWARE_Wait(1000);
	
	send_command(at_command_at);
	TIMER_SOFTWARE_Wait(1000);
	
	TIMER_SOFTWARE_reset_timer(handler_main);
	TIMER_SOFTWARE_start_timer(handler_main);
	
	for(i = 0; i <= 40; i++) {
		for(j = 0; j <= LCD_WIDTH; j++) {
			DRV_LCD_PutPixel(i, j, primary.red, primary.green, primary.blue);
		}
	}
	
	while (1) {	
				
		if (TIMER_SOFTWARE_interrupt_pending(handler_main)) {
			//printf("Sunt aici\n");
			execute_command(at_command_csq, AT_CSQ);
			//print_data();
			if (verify_response(&data)) {
				rssi_value_asu = get_asu_from_response(&data);
				rssi_value_dbmw = asu_to_dbmw(rssi_value_asu);
				//printf("GSM modem signal %"PRIu32" ASU -> %"PRIi32" dBmW\n", rssi_value_asu, rssi_value_dbmw);
				sprintf(s, "GSM modem signal %"PRIi32" dBmW", rssi_value_dbmw);
				DRV_LCD_Puts(s, 20, 5, white, primary, 0);
			}
			/*
			execute_command(at_command_gsn, AT_GSN);
			if (verify_response(&data)) {
				printf("Modem IMEI: %s\n", get_imei(&data));
			}
			*/
			execute_command(at_command_extended_creg_1, AT_CREG);
			execute_command(at_command_creg, AT_CREG);
			if(verify_response(&data)){
				//printf("Network state: %s\n", get_network_state(&data));
				sprintf(s, "Network state: %s", get_network_state(&data));
				DRV_LCD_Puts(s, 20, 18, white, primary, 0);
			}
			
			execute_command(at_command_cops, AT_COPS);
			if(verify_response(&data)) {
			//	print_data();
				//printf("Network operator name: %s\n", get_operator_name(&data));
				sprintf(s, "Network operator name: %s", get_operator_name(&data));
				DRV_LCD_Puts(s, 20, 30, white, primary, 0);
			}
			/*
			execute_command(at_command_gmi, AT_GMI);
			if(verify_response(&data)) {
				printf("Modem manufacturer: %s\n", get_manufacturer_identity(&data));
			}
			
			execute_command(at_command_gmr, AT_GMR);
			if(verify_response(&data)) {
				printf("Modem software version: %s\n", get_software_version(&data));
			}
			*/
			/*
			execute_command(at_command_extended_creg_2, AT_CREG);
			print_data();
			execute_command(at_command_creg, AT_CREG);
			print_data();
			*/
			

			TIMER_SOFTWARE_clear_interrupt(handler_main);

		//	printf("\n");
		} 
		DRV_TOUCHSCREEN_Process();
	}
}
