#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
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

int main(void)
{
	uint8_t ch;
	BoardInit();
	testLCD();
	
	DRV_UART_Configure(UART_3, UART_CHARACTER_LENGTH_8, 115200, UART_PARITY_NO_PARITY, 1, TRUE, 3);
	DRV_UART_Configure(UART_2, UART_CHARACTER_LENGTH_8, 115200, UART_PARITY_NO_PARITY, 1, TRUE, 3);
	
	while(1)
	{
		DRV_LED_Toggle(LED_4);
		
	}
	/*
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
*/
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
	
	return 0; 
}
