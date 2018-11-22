#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
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

timer_software_handler_t handler;

void timer_callback_1(timer_software_handler_t h)
{
}

/*
void testLCD()
{
	uint32_t i,j;
	LCD_PIXEL foreground = {0, 255, 0, 0};
	LCD_PIXEL background = {0, 0, 0, 0};
	
	
	for (i = 0; i < LCD_HEIGHT; i++)
	{
		for (j = 0; j < LCD_WIDTH / 3; j++)
		{
			DRV_LCD_PutPixel(i, j, 255, 0, 0);
		}
		for (j = LCD_WIDTH / 3; j < 2 * (LCD_WIDTH / 3); j++)
		{
			DRV_LCD_PutPixel(i, j, 230, 220, 0);
		}
		for (j = 2 * (LCD_WIDTH / 3); j < LCD_WIDTH; j++)
		{
			DRV_LCD_PutPixel(i, j, 0, 0, 255);
		}
	}

	DRV_LCD_Puts("Hello", 20, 30, foreground, background, TRUE);
	DRV_LCD_Puts("Hello", 20, 60, foreground, background, FALSE);	
}
*/
void TouchScreenCallBack(TouchResult* touchData)
{
	printf("touched X=%3d Y=%3d\n", touchData->X, touchData->Y);		
	
}

void BoardInit()
{
	timer_software_handler_t handler;
	
	TIMER_SOFTWARE_init_system();
	
	
	DRV_SDRAM_Init();
	
	initRetargetDebugSystem();
	DRV_LCD_Init();
	DRV_LCD_ClrScr();
	DRV_LCD_PowerOn();	
	
	DRV_TOUCHSCREEN_Init();
	DRV_TOUCHSCREEN_SetTouchCallback(TouchScreenCallBack);
	DRV_LED_Init();
	printf ("Hello\n");	
	
	handler = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(handler, MODE_1, 100, 1);
	TIMER_SOFTWARE_set_callback(handler, timer_callback_1);
	TIMER_SOFTWARE_start_timer(handler);
}

void send_command(char *cmd) {
	//uint8_t ch;
	
	DRV_UART_FlushRX(UART_3);
	DRV_UART_FlushTX(UART_3);
	//printf("execute_command\n");
	DRV_UART_Write(UART_3, (uint8_t*) cmd, strlen(cmd)); 
	
	/*while(1) {
		while(DRV_UART_ReadByte(UART_3, &ch) == OK) {
			printf("%c", ch);
		}
	}*/
}

void get_command_response() {
	uint8_t ch;
	
	TIMER_SOFTWARE_reset_timer(handler);
	TIMER_SOFTWARE_start_timer(handler);
	
	while(!TIMER_SOFTWARE_interrupt_pending(handler)){
		
		while(DRV_UART_BytesAvailable(UART_3) > 0) {
			DRV_UART_ReadByte(UART_3, &ch);
			printf("%c", ch);
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
uint32_t get_asu_from_response(AT_DATA *data){
	return 0;
}
uint32_t asu_to_dbmw(uint32_t asu) {
	return 2*asu - 113;
}
int main(void)
{
	uint8_t ch;
	uint32_t rssi_value_asu;
	uint32_t rssi_value_dbmw;
	handler = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(handler, MODE_1, 1000, 1);
	
	BoardInit();
	//testLCD();
	
	DRV_UART_Configure(UART_3, UART_CHARACTER_LENGTH_8, 115200, UART_PARITY_NO_PARITY, 1, TRUE, 3);
	//DRV_UART_Configure(UART_2, UART_CHARACTER_LENGTH_8, 115200, UART_PARITY_NO_PARITY, 1, TRUE, 3);
	
	
	// send AT command to tell modem to autobaud
	send_command(at_command_at);
	TIMER_SOFTWARE_Wait(1000);
	
	send_command(at_command_at);
	TIMER_SOFTWARE_Wait(1000);
	
	send_command(at_command_at);
	TIMER_SOFTWARE_Wait(1000);
	
	TIMER_SOFTWARE_reset_timer(handler);
	TIMER_SOFTWARE_start_timer(handler);
	
	while(1) {
		
		if(TIMER_SOFTWARE_interrupt_pending(handler)) {
			execute_command(at_command_csq);
			if(verify_response(&data)) {
				rssi_value_asu = get_asu_from_response(&data);
				rssi_value_dbmw = asu_to_dbmw(rssi_value_asu);
				printf("GSM modem signal %"PRIu32" ASU -> %"PRIu32" dBmW\n", rssi_value_asu, rssi_value_dbmw);
			}
			TIMER_SOFTWARE_clear_interrupt(handler);
		}
		
	}
	
	
	
/*	while(1)
	{
		DRV_LED_Toggle(LED_4);
		
	}
	
	while(1)
	{
		DRV_UART_SendByte(UART_3, 'A');
	//	TIMER_SOFTWARE_Wait(1000);
	}
	*/
	/*
	while(1)
	{
		if (DRV_UART_ReadByte(UART_3, &ch) == OK)
		{
			DRV_UART_SendByte(UART_3, ch);
		}		
	}

	while(1)
	{
		if (DRV_UART_ReadByte(UART_0, &ch) == OK)
		{
			DRV_UART_SendByte(UART_3, ch);
		}
		if (DRV_UART_ReadByte(UART_3, &ch) == OK)
		{
			DRV_UART_SendByte(UART_0, ch);
		}
		if (DRV_UART_ReadByte(UART_2, &ch) == OK)
		{
			DRV_UART_SendByte(UART_0, ch);
		}
	}
	
	while(1)
	{
		DRV_UART_Process();
		DRV_TOUCHSCREEN_Process();
	}
*/
	return 0; 
}






























