//*****************************************************************************
//! \file	drv_lcd.h
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	LCD Driver
//! 
//! Contains the driver for the LCD on the Olimex board
//! The LCD on the Olimex Board is a RGB LCD, resolution 480x272.
//! Before using any functions from this module, a call of DRV_LCD_Init must be performed
//*****************************************************************************

#ifndef __DRV_LCD_H
#define __DRV_LCD_H

#include <stdint.h>
#include "drv_general.h"

//*****************************************************************************
//! \addtogroup DRV_LCD
//! @{
//*****************************************************************************

/*! 
	LCD horizontal size
*/

#define LCD_WIDTH	480 

/*! 
	LCD vertical size
*/
#define LCD_HEIGHT	272

#define C_GLCD_REFRESH_FREQ     (50HZ)
#define C_GLCD_H_SIZE           LCD_WIDTH //480       //
#define C_GLCD_H_PULSE          45        //
#define C_GLCD_H_FRONT_PORCH    45        //45
#define C_GLCD_H_BACK_PORCH     45       //45
#define C_GLCD_V_SIZE           LCD_HEIGHT//272       //
#define C_GLCD_V_PULSE          16        //
#define C_GLCD_V_FRONT_PORCH    16        //16
#define C_GLCD_V_BACK_PORCH     16        //16
#define C_GLCD_CLK_PER_LINE     (C_GLCD_H_SIZE + C_GLCD_H_PULSE + C_GLCD_H_FRONT_PORCH + C_GLCD_H_BACK_PORCH)
#define C_GLCD_LINES_PER_FRAME  (C_GLCD_V_SIZE + C_GLCD_V_PULSE + C_GLCD_V_FRONT_PORCH + C_GLCD_V_BACK_PORCH)
#define C_GLCD_PIX_CLK          (6.4MHZ)
#define C_GLCD_PWR_ENA_DIS_DLY  10000000
#define C_GLCD_ENA_DIS_DLY      10000000


//*****************************************************************************
//! \struct LCD_PIXEL
//! The structure used for encapsulating a RGB 888 pixel.
//! 
//*****************************************************************************

typedef struct
{	
	uint8_t red; /*!< red component of the color*/	
	uint8_t green; /*!< green component of the color */	
	uint8_t blue; /*!< blue component of the color */	
	uint8_t padding;/*!< padding byte in order to meet the DMA requirements, pixel size 32 bit (24 bit usable data) */
}LCD_PIXEL;

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//extern LCD_PIXEL* LCD_Frame_Buffer;
extern LCD_PIXEL LCD_Frame_Buffer[LCD_WIDTH * LCD_HEIGHT];

void DRV_LCD_Init(void);
void DRV_LCD_PowerOn(void);
void DRV_LCD_PowerOff(void);
void DRV_LCD_TestFillColor(uint8_t red, uint8_t green, uint8_t blue);
void DRV_LCD_PutPixel(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue);
void DRV_LCD_ClrScr(void);

void DRV_LCD_Puts(char *string, uint16_t x, uint16_t y, LCD_PIXEL TextColor, LCD_PIXEL BackgroundTextColor, BOOLEAN big);

#endif
