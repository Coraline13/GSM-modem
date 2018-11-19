//*****************************************************************************
//! \file	drv_led.c
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	LED Driver
//! 
//! Contains the driver for the 4 leds on the board
//*****************************************************************************

//*****************************************************************************
//! \headerfile drv_led.h "drv_led.h"
//*****************************************************************************

//*****************************************************************************
//! \addtogroup DRV_LED
//! @{
//! \brief	LED Driver
//! 
//! Contains the driver for the 4 leds on the board
//*****************************************************************************

#include "drv_led.h"
#include "LPC177x_8x.h"
#include "system_LPC177x_8x.h"
#include "drv_general.h"


//*****************************************************************************
/*! \var LED_MAP LedMap[]
	\brief LED Mapping Table
*/
//*****************************************************************************
LED_MAP LedMap[] = 	{
						{LED_UNKNOWN,		LED_OFF,		0},
						{LED_1,				LED_OFF,		11},
						{LED_2,				LED_OFF,		10},
						{LED_3,				LED_OFF,		1},
						{LED_4,				LED_OFF,		0},
					};

//*****************************************************************************
//! Initialize the LED driver
//!
//! This function is used to initialize the structures of the led driver as well as to configure the pins where the leds are connected.
//! \return None.
//*****************************************************************************
void DRV_LED_Init()
{
	uint8_t i = 0;
	LPC_SC->PCONP |= 1 << 15;
	LPC_IOCON->P0_11 = 0;
	LPC_IOCON->P0_10 = 0;
	LPC_IOCON->P0_1 = 0;
	LPC_IOCON->P0_0 = 0;
	for (i = LED_1; i <= LED_4; i++)
	{
		LPC_GPIO0->DIR |= 1 << LedMap[i].led_pin;
		DRV_LED_Off(LedMap[i].led_number);
	}
}

//*****************************************************************************
//! Turn LED Off
//!
//! This function is used switch the led off
//! \param led The led to be switched off
//! \return None.
//*****************************************************************************
void DRV_LED_Off(LED led)
{
	if (led > LED_4)
	{
		return;
	}
	LPC_GPIO0->SET = 1 << LedMap[led].led_pin;
	LedMap[led].led_state = LED_OFF;
}

//*****************************************************************************
//! Turn LED On
//!
//! This function is used switch the led on
//! \param led The led to be switched on
//! \return None.
//*****************************************************************************
void DRV_LED_On(LED led)
{
	if (led > LED_4)
	{
		return;
	}
	LPC_GPIO0->CLR = 1 << LedMap[led].led_pin;
	LedMap[led].led_state = LED_ON;
}

//*****************************************************************************
//! Toggle the led
//!
//! This function is used toggled the led. If the led is off it will be switched on and viceversa.
//! \param led The led to be toggled
//! \return None.
//*****************************************************************************
void DRV_LED_Toggle(LED led)
{
	if (led > LED_4)
	{
		return;
	}
	if (LedMap[led].led_state == LED_ON)
	{
		DRV_LED_Off(led);
	}
	else
	{
		DRV_LED_On(led);
	}
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
