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
char at_command_creg[] = "AT+CREG?\r\n";
char at_command_cops[] = "AT+COPS?\r\n";
char at_command_gsn[] = "AT+GSN\r\n";
char at_command_gmi[] = "AT+GMI\r\n";
char at_command_gmr[] = "AT+GMR\r\n";
char at_command_extended_creg_1[] = "AT+CREG=1\r\n";
char at_command_extended_creg_2[] = "AT+CREG=2\r\n";

timer_software_handler_t handler_main;
timer_software_handler_t handler_get_response;

void timer_callback_1(timer_software_handler_t h)
{
}

void TouchScreenCallBack(TouchResult* touchData)
{
	printf("touched X=%3d Y=%3d\n", touchData->X, touchData->Y);	
}

void BoardInit()
{	
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

char* get_operator_name(AT_DATA *data) { // pentru AT_COPS
	char *op_name;
	op_name = strstr(&data->data[0][8], ",");
	return op_name;
}

char* get_imei(AT_DATA *data) { // pentru AT_GSN
	return data->data[0];
}

char* get_manufacturer_identity(AT_DATA *data) { // pentru AT_GMI
	return data->data[0];
}

char* get_software_version(AT_DATA *data) { // pentru AT_GMR
	return &data->data[0][10];
}

char* get_network_state(AT_DATA *data) { // pentru AT_CREG
	uint8_t stat;
	stat = strtol(&data->data[0][9], NULL, 10);
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
	
	while (1) {
		
		if (TIMER_SOFTWARE_interrupt_pending(handler_main)) {
			execute_command(at_command_csq, AT_CSQ);
			if (verify_response(&data)) {
				rssi_value_asu = get_asu_from_response(&data);
				rssi_value_dbmw = asu_to_dbmw(rssi_value_asu);
				printf("GSM modem signal %"PRIu32" ASU -> %"PRIi32" dBmW\n", rssi_value_asu, rssi_value_dbmw);
			}
			
			execute_command(at_command_gsn, AT_GSN);
			if (verify_response(&data)) {
				printf("Modem IMEI: %s\n", get_imei(&data));
			}
			
			execute_command(at_command_extended_creg_1, AT_CREG);
			execute_command(at_command_creg, AT_CREG);
			if(verify_response(&data)){
				printf("Network state: %s\n", get_network_state(&data));
			}
			
			execute_command(at_command_cops, AT_COPS);
			if(verify_response(&data)) {
				printf("Network operator name: %s\n", get_operator_name(&data));
			}
			
			execute_command(at_command_gmi, AT_GMI);
			if(verify_response(&data)) {
				printf("Modem manufacturer: %s\n", get_manufacturer_identity(&data));
			}
			
			execute_command(at_command_gmr, AT_GMR);
			if(verify_response(&data)) {
				printf("Modem software version: %s\n", get_software_version(&data));
			}
			/*
			execute_command(at_command_extended_creg_2, AT_CREG);
			print_data();
			execute_command(at_command_creg, AT_CREG);
			print_data();
			*/
			TIMER_SOFTWARE_clear_interrupt(handler_main);
			printf("\n");
		}
	}
}
