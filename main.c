#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "LPC177x_8x.h"
#include "system_LPC177x_8x.h"
#include <retarget.h>
#include <at.h>

#include <DRV\drv_sdram.h>
#include <DRV\drv_lcd.h>
#include <DRV\drv_uart.h>
#include <DRV\drv_touchscreen.h>
#include <DRV\drv_led.h>
#include <utils\timer_software.h>
#include <utils\timer_software_init.h>
#include <string.h>
#include <stdlib.h>


const char at_command_simple[5]="AT\r\n";



const char at_command_csq[9] = "AT+CSQ\r\n";

const char at_command_creg[11] = "AT+CREG?\r\n"; //State of network registration 
const char at_command_cops[11] = "AT+COPS?\r\n"; //Name of network operator 
const char at_command_gsn[9] = "AT+GSN\r\n"; //Modem IMEI
const char at_command_gmi[9] = "AT+GMI\r\n"; //Modem Manufacturer 
const char at_command_gmr[9] = "AT+GMR\r\n"; //Modem Software Version
const char at_command_cregExt[10] = "AT+CREG\r\n"; //Cell ID and Location ID  (extended creg -> optional)

timer_software_handler_t handler;
timer_software_handler_t handler_timeout;
STATE_MACHINE_RETURN_VALUE ret;


void timer_callback_1(timer_software_handler_t h)
{
}

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

void TouchScreenCallBack(TouchResult* touchData)
{
	printf("touched X=%3d Y=%3d\n", touchData->X, touchData->Y);		
	
}

void BoardInit()
{	
	TIMER_SOFTWARE_init_system();
	DRV_SDRAM_Init();
	initRetargetDebugSystem();
}

void my_callback(timer_software_handler_t handler){
	printf("Cool!\n");
}


void SendCommand(const char *command)
{
 DRV_UART_FlushRX(UART_3);
 DRV_UART_FlushTX(UART_3);
 DRV_UART_Write(UART_3, (uint8_t *)command, strlen(command));
} 

void GetCommandResponse()
{
 uint8_t ch;
 BOOLEAN ready = FALSE;
 TIMER_SOFTWARE_reset_timer(handler_timeout);
 TIMER_SOFTWARE_start_timer(handler_timeout);

 while ((!TIMER_SOFTWARE_interrupt_pending(handler_timeout)) && (ready == FALSE))
 {
	 while (DRV_UART_BytesAvailable(UART_3) > 0)
	 {
			DRV_UART_ReadByte(UART_3, &ch);
			printf("%c", ch);
		 	TIMER_SOFTWARE_Wait(100);
			ret = at_command_parse(ch);
			 if (ret != STATE_MACHINE_NOT_READY)
			 {
				ready = TRUE;
			 }
	 }	
	 
 }
// printf("eW\n");
// TIMER_SOFTWARE_Wait(1000);
 //print_AT_commands();
// TIMER_SOFTWARE_Wait(1000);
 //printf("c%d\n", info.line_count);
 //TIMER_SOFTWARE_Wait(1000);
 //printf("f->%c\n", info.data[0][0]);
} 

void ExecuteCommand(const char *command)
{
 SendCommand(command);
 GetCommandResponse();
} 

uint32_t ConvertAsuToDbmw(uint32_t rssi_value_asu)
{
	return 2 * rssi_value_asu - 113;
}
	
uint32_t ExtractData()
{
	uint8_t *data = info.data[info.line_count - 1];
	data = data + 5;
	return atoi((const char *)data);
}

float ExtractDataCREG()
{
	uint8_t *data = info.data[info.line_count - 1];
	uint8_t *comma_start;
	uint32_t nr_aft;
	data = data + 6;
	comma_start = (uint8_t *)strchr((char *)data,',');
	comma_start [0] = '.'; //replace with . to use atof
	return atof((const char *)data);
}

char *ExtractDataGSN(){
	
}


BOOLEAN CommandResponseValid(){
	if(ret == STATE_MACHINE_READY_OK){
		printf("isok\n");
		TIMER_SOFTWARE_Wait(100); 
		return true;
	}
	else return false;
}	
int main(void)
{
	// variables

	uint32_t rssi_value_asu;
	uint32_t rssi_value_dbmw;
	float creg_resp;	
	uint8_t ch;
	
	// initialisation board, lcd, led, uart
	BoardInit(); 
	//testLCD();
	//DRV_LED_Init();	

	DRV_UART_Configure(UART_3, UART_CHARACTER_LENGTH_8, 115200, UART_PARITY_NO_PARITY, 1, TRUE, 3);
	DRV_UART_Configure(UART_0, UART_CHARACTER_LENGTH_8, 115200, UART_PARITY_NO_PARITY, 1, TRUE, 3);

	//printf("c\n");
	
	// configure BAUT rate
	 //send the AUTOBAUT command 3 times to be sure
	DRV_UART_Write(UART_3, (uint8_t *) at_command_simple, strlen(at_command_simple));
	TIMER_SOFTWARE_Wait(1000); 
	
	DRV_UART_Write(UART_3, (uint8_t *)at_command_simple, strlen(at_command_simple));
	TIMER_SOFTWARE_Wait(1000); 
	
	DRV_UART_Write(UART_3, (uint8_t *)at_command_simple, strlen(at_command_simple));
	TIMER_SOFTWARE_Wait(1000); 

	//printf("wait\n");
	

	handler = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(handler, MODE_1, 2000, true); 

	handler_timeout = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(handler_timeout, MODE_1, 30000, true);  
	
	//printf("init\n");
	TIMER_SOFTWARE_Wait(100);
	
	TIMER_SOFTWARE_start_timer(handler); 
	while (1) //pending interrupt // every 5 sec
	 {
		 if (TIMER_SOFTWARE_interrupt_pending(handler))
		 {
			 //printf("inter\n");
			 TIMER_SOFTWARE_Wait(1000);
			 //FIRST COMMAND
			 ExecuteCommand(at_command_csq);
			 if (CommandResponseValid())
			 {
				 //printf("ok\n");
				 TIMER_SOFTWARE_Wait(1000);
				 rssi_value_asu = ExtractData();
				 //printf("v%d\n",rssi_value_asu);
				 TIMER_SOFTWARE_Wait(1000);
				rssi_value_dbmw = ConvertAsuToDbmw(rssi_value_asu);
				printf("GSM Modem signal  %d ASU -> %d dBmW.\n", rssi_value_asu, rssi_value_dbmw);
			 }
			 //printf("done\n");
			 TIMER_SOFTWARE_Wait(1000);
			 //SECOND COMMAND
			 ExecuteCommand(at_command_creg);
			 TIMER_SOFTWARE_Wait(100);
			 if (CommandResponseValid())
			 {
				 TIMER_SOFTWARE_Wait(1000);
				 creg_resp = ExtractDataCREG();
				 printf("creg->%f",creg_resp);
				 TIMER_SOFTWARE_Wait(1000);
			 }
			 TIMER_SOFTWARE_Wait(1000);
			 //THIRD COMMAND
			 ExecuteCommand(at_command_cops);
			 TIMER_SOFTWARE_Wait(100);
			 if (CommandResponseValid())
			 {
				 printf("yes\n");
				 TIMER_SOFTWARE_Wait(100);
			 }
			  TIMER_SOFTWARE_Wait(1000);
			 
			 //FOURTH COMMAND 
			 ExecuteCommand(at_command_gsn);
			 TIMER_SOFTWARE_Wait(100);
			 special_command = 1;
			 if (CommandResponseValid())
			 {
				 ExtractDataGSN();
				 printf("yes gsn\n");
				 TIMER_SOFTWARE_Wait(100);
			 }
			 special_command = 0;
			 //FIFTH COMMAND
			 
			 TIMER_SOFTWARE_clear_interrupt(handler);
		 }
	 } 
}
