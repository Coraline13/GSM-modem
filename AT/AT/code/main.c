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

char at_command_at[] = "AT\r\n";
char at_command_csq[] = "AT+CSQ\r\n";
char at_command_creg[] = "AT+CREG\r\n";
char at_command_cops[] = "AT+COPS\r\n";
char at_command_gsn[] = "AT+GSN\r\n";
char at_command_gmi[] = "AT+GMI\r\n";
char at_command_gmr[] = "AT+GMR\r\n";
// char at_command_extended_creg[] = "AT+CREG\r\n";

timer_software_handler_t handler;

void timer_callback_1(timer_software_handler_t h)
{
}

void TouchScreenCallBack(TouchResult* touchData)
{
	printf("touched X=%3d Y=%3d\n", touchData->X, touchData->Y);	
}

void BoardInit()
{
	// timer_software_handler_t handler;
	
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
}

void send_command(char *cmd) {
	uint8_t ch;
	
	DRV_UART_FlushRX(UART_3);
	DRV_UART_FlushTX(UART_3);
	DRV_UART_Write(UART_3, (uint8_t*) cmd, strlen(cmd)); 
}

void get_command_response() {
	uint8_t ch;
	uint8_t state = 0;
	handler = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(handler, MODE_1, 10000, 1);
	
	TIMER_SOFTWARE_reset_timer(handler);
	TIMER_SOFTWARE_start_timer(handler);
	
	while (!TIMER_SOFTWARE_interrupt_pending(handler)){
		
		while (DRV_UART_BytesAvailable(UART_3) > 0 && (state != ERROR_STATE)) {
			DRV_UART_ReadByte(UART_3, &ch);
			state = parse(ch, AT_CSQ);
		}
	}
}

void execute_command(char *cmd) {
	send_command(cmd);
	get_command_response();
}

bool verify_response(AT_DATA *data) {	
	return data->ok;
}

uint32_t tol(char *s) {
	uint32_t nr = 0, i = 0;

	while (s[i] != 0) {
		if (s[i] >= '0' && s[i] <= '9') {
			nr = nr * 10 + (s[i] - '0');
		}
		if (s[i] == ',') {
			return nr;
		}
		i++;
	}
	return nr;
}

uint32_t get_asu_from_response(AT_DATA *data){
	char *text;
	// printf("%ld\n", tol(data->data[0]));
	// return tol(data->data[0]);
	return strtol(&data->data[0][6], NULL, 10);
}
int32_t asu_to_dbmw(uint32_t asu) {
	return 2 * asu - 113;
}

char* get_operator_name(AT_DATA *data) { // pentru AT_COPS
	char *op_name;
	op_name = strstr(&data->data[0][8], ",");
	return op_name;
}

char* get_imei(AT_DATA *data) { // pentru AT_GSN
	return &data->data[0];
}

char* get_manufacturer_identity(AT_DATA *data) { // pentru AT_GMI
	return &data->data[0];
}

char* get_software_version(AT_DATA *data) { // pentru AT_GMR
	return &data->data[0][8];
}

char* get_network_state(AT_DATA *data) { // pentru AT_CREG
	uint8_t stat;
	stat = strtol(&data->data[0][8], NULL, 10);
	switch (stat) {
	case 0: return "Modem is not registered in the network and is not searching for a network";
	case 1: return "Modem is registered to home network";
	case 2: return "Modem is not registered but it is currently searching for a network";
	case 3: return "Modem registration into the network was denied";
	case 4: return "Unknown modem registration state";
	case 5: return "Modem is registered to roaming network";
	default: return "Unknown state";
	}
}

int main(void)
{
	uint32_t rssi_value_asu;
	uint32_t rssi_value_dbmw;
	
	BoardInit();
	// testLCD();
	
	DRV_UART_Configure(UART_3, UART_CHARACTER_LENGTH_8, 115200, UART_PARITY_NO_PARITY, 1, TRUE, 3);
	
	handler = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(handler, MODE_1, 1000, 1);
	// send AT command to tell modem to autobaud
	send_command(at_command_at);
	TIMER_SOFTWARE_Wait(1000);
	
	send_command(at_command_at);
	TIMER_SOFTWARE_Wait(1000);
	
	send_command(at_command_at);
	TIMER_SOFTWARE_Wait(1000);
	
	TIMER_SOFTWARE_reset_timer(handler);
	TIMER_SOFTWARE_start_timer(handler);
	
	while (1) {
		
		if (TIMER_SOFTWARE_interrupt_pending(handler)) {
			execute_command(at_command_csq);
			print_data();
			if (verify_response(&data)) {
				rssi_value_asu = get_asu_from_response(&data);
				rssi_value_dbmw = asu_to_dbmw(rssi_value_asu);
				printf("GSM modem signal %"PRIu32" ASU -> %"PRIi32" dBmW\n", rssi_value_asu, rssi_value_dbmw);
			}
			TIMER_SOFTWARE_clear_interrupt(handler);
		}
	}
	
	