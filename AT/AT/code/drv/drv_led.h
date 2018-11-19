//*****************************************************************************
//! \file	drv_led.h
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	LED Driver
//! 
//! Contains the driver for the 4 leds on the board
//*****************************************************************************
#ifndef __DRV_LED
#define __DRV_LCD



#include <stdint.h>
#include <utils/ringbuf.h>
#include <utils/timer_software.h>
#include "drv_general.h"

//*****************************************************************************
//! \addtogroup DRV_LED
//! @{
//*****************************************************************************

//*****************************************************************************
//! \enum LED_STATE
//! \typedef LED_STATE
//! Identify the led state
//*****************************************************************************
typedef enum
{
	LED_OFF,	/**< Led on  */
	LED_ON,		/**< Led off  */
}LED_STATE;

//*****************************************************************************
//! \enum LED
//! \typedef LED
//! Identify the leds
//*****************************************************************************
typedef enum
{
	LED_UNKNOWN = 0,	/**< Unknown Led  */
	LED_1 = 1,			/**< Led 1  */
	LED_2 = 2,			/**< Led 2  */
	LED_3 = 3,			/**< Led 3  */
	LED_4 = 4,			/**< Led 4  */
}LED;

//*****************************************************************************
//! \struct LED_MAP
//! Structure that holds various information about each led
//! The structure is initialized internally for each led. No user intervention is required
//! \private
//*****************************************************************************
typedef struct
{
	LED led_number;				/**< Identifies the led */
	LED_STATE led_state;		/**< Specifies the current led state */
	uint8_t led_pin;			/**< Specifies the pin at which the led is connected on Port0 */
}LED_MAP;

void DRV_LED_Init(void);
void DRV_LED_Off(LED led);
void DRV_LED_On(LED led);
void DRV_LED_Toggle(LED led);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#endif
