//*****************************************************************************
//! \file	drv_lcd.c
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	LCD Driver
//! 
//! Contains the driver for the LCD on the Olimex board
//! The LCD on the Olimex Board is a RGB LCD, resolution 480x272.
//! Before using any functions from this module, a call of DRV_LCD_Init must be performed
//*****************************************************************************

//*****************************************************************************
//! \headerfile drv_lcd.h "drv_lcd.h"
//*****************************************************************************

//*****************************************************************************
//! \addtogroup DRV_LCD
//! @{
//! \brief	LCD Driver
//! 
//! Contains the driver for the LCD on the Olimex board
//! The LCD on the Olimex Board is a RGB LCD, resolution 480x272.
//! Before using any functions from this module, a call of DRV_LCD_Init must be performed
//*****************************************************************************

#include "drv_lcd.h"
#include <stdint.h>
#include <assert.h>
#include "LPC177x_8x.h"
#include "system_LPC177x_8x.h"
#include "drv_general.h"
#include "gl_fonts.h"

#define GL_FONT_BIG           0x00
#define GL_FONT_SMALL         0x01
#define GL_FONT_BIG_WIDTH       16
#define GL_FONT_BIG_HEIGHT      24
#define GL_FONT_SMALL_WIDTH     8
#define GL_FONT_SMALL_HEIGHT    8

//*****************************************************************************
/*! \var LCD_PIXEL* LCD_Frame_Buffer
	\brief The LCD Frame Buffer
*/
//*****************************************************************************
LCD_PIXEL LCD_Frame_Buffer[LCD_WIDTH * LCD_HEIGHT];

//*****************************************************************************
//! Initialize the LCD Controller
//!
//! This function is used to initialize the LCD controller of the processor to use the LCD on the board
//! The frame buffer for the LCD is stored on the external RAM memory
//! \return None.
//*****************************************************************************
void DRV_LCD_Init()
{
	
	// Assign pins
	LPC_IOCON->P0_4 = 0x27;
	LPC_IOCON->P0_5 = 0x27;
	LPC_IOCON->P0_6 = 0x27;
	LPC_IOCON->P0_7 = 0x27;
	LPC_IOCON->P0_8 = 0x27;
	LPC_IOCON->P0_9 = 0x27;
	LPC_IOCON->P1_20 = 0x27;
	LPC_IOCON->P1_21 = 0x27;
	LPC_IOCON->P1_22 = 0x27;
	LPC_IOCON->P1_23 = 0x27;
	LPC_IOCON->P1_24 = 0x27;
	LPC_IOCON->P1_25 = 0x27;
	LPC_IOCON->P1_26 = 0x27;
	LPC_IOCON->P1_27 = 0x27;
	LPC_IOCON->P1_28 = 0x27;
	LPC_IOCON->P1_29 = 0x27;
	//  LPC_IOCON->P2_00 = 0x27;
	LPC_IOCON->P2_1 = 0x20;
	LPC_IOCON->P2_2 = 0x27;
	LPC_IOCON->P2_3 = 0x27;
	LPC_IOCON->P2_4 = 0x27;
	LPC_IOCON->P2_5 = 0x27;
	LPC_IOCON->P2_6 = 0x27;
	LPC_IOCON->P2_7 = 0x27;
	LPC_IOCON->P2_8 = 0x27;
	LPC_IOCON->P2_9 = 0x27;
	//  LPC_IOCON->P2_11 = 0x27;
	LPC_IOCON->P2_12 = 0x27;
	LPC_IOCON->P2_13 = 0x27;
	LPC_IOCON->P4_28 = 0x27;
	LPC_IOCON->P4_29 = 0x27;
	
	  /*Back light enable*/
	LPC_GPIO2->DIR |= 2;
	LPC_GPIO2->SET = 2;

	LPC_SC->PCONP |= 1;// enable LCD controller clock
	
	LPC_LCD->CRSR_CTRL = 0;											// Disable cursor
	LPC_LCD->CTRL = 0; 													// disable GLCD controller
	LPC_LCD->CTRL |= 5 << 1; 											// 24bpp
	LPC_LCD->CTRL |= 1 << 5;											// TFT panel
	LPC_LCD->CTRL &= ~(1 << 7);											// single panel
	LPC_LCD->CTRL &= ~(1 << 8); 										// normal output
	LPC_LCD->CTRL &= ~(1 << 9);											// little endian byte order
	LPC_LCD->CTRL &= ~(1 << 10);										// little endian pix order
	LPC_LCD->CTRL &= ~(1 << 11);										// disable power
	LPC_SC->LCD_CFG = (PeripheralClock / (uint32_t)(C_GLCD_PIX_CLK));	// init pixel clock	
	LPC_LCD->POL |= 1 << 26; 											// bypass inrenal clk divider
	LPC_LCD->POL &= ~(1 << 5);											// clock source for the LCD block is HCLK
	LPC_LCD->POL |= 1 << 11;											// LCDFP pin is active LOW and inactive HIGH
	LPC_LCD->POL |= 1 << 12;											// LCDLP pin is active LOW and inactive HIGH
	LPC_LCD->POL |= 1 << 13;											// data is driven out into the LCD on the falling edge
	LPC_LCD->POL &= ~(1 << 14);											// active high
	LPC_LCD->POL |= (C_GLCD_H_SIZE - 1) << 16;
	// init Horizontal Timing
	LPC_LCD->TIMH = 0;
	LPC_LCD->TIMH |= (C_GLCD_H_BACK_PORCH - 1) << 24;
	LPC_LCD->TIMH |= (C_GLCD_H_FRONT_PORCH - 1) << 16;
	LPC_LCD->TIMH |= (C_GLCD_H_PULSE - 1) << 8;
	LPC_LCD->TIMH |= ((C_GLCD_H_SIZE/16) - 1) << 2;
	// init Vertical Timing
	LPC_LCD->TIMV = 0;
	LPC_LCD->TIMV |= C_GLCD_V_BACK_PORCH << 24;
	LPC_LCD->TIMV |= C_GLCD_V_FRONT_PORCH << 16;
	LPC_LCD->TIMV |= C_GLCD_V_PULSE << 8;
	LPC_LCD->TIMV |= C_GLCD_V_SIZE - 1;
	// Frame Base Address doubleword aligned
	LPC_LCD->UPBASE = ((uint32_t)(LCD_Frame_Buffer)) & (~7U);
	LPC_LCD->LPBASE = ((uint32_t)(LCD_Frame_Buffer)) & (~7U);
}

//*****************************************************************************
//! Power on the LCD
//!
//! This function is used to power on the LCD
//! \return None.
//! \private
//*****************************************************************************
void DRV_LCD_PowerOn()
{
	volatile uint32_t i;
	LPC_LCD->CTRL |= 1;
	for (i = C_GLCD_PWR_ENA_DIS_DLY; i; i--);
	LPC_LCD->CTRL |= 1 << 11;
}

//*****************************************************************************
//! Power off the LCD
//!
//! This function is used to power off the LCD
//! \return None.
//! \private
//*****************************************************************************
void DRV_LCD_PowerOff()
{
	volatile uint32_t i;
	LPC_LCD->CTRL &= ~1;
	for (i = C_GLCD_PWR_ENA_DIS_DLY; i; i--);
	LPC_LCD->CTRL &= ~(1 << 11);
}

//*****************************************************************************
//! Fill the LCD
//!
//! This function is used to fill the whole LCD with a given color
//! \param red The red component of the color
//! \param green The green component of the color
//! \param blue The blue component of the color
//! \return None.
//*****************************************************************************
void DRV_LCD_TestFillColor(uint8_t red, uint8_t green, uint8_t blue)
{
	uint32_t i;
	LCD_PIXEL *p = LCD_Frame_Buffer;
	
	for (i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++)
	{
		p->red = red;
		p->green = green;
		p->blue = blue;
		p->padding = 0;
		p++;
	}
}

//*****************************************************************************
//! Draw pixel
//!
//! This function is used to draw a pixel at the specified coordinates of a given color
//! \param x x coordinate (x < LCD_HEIGHT)
//! \param y y coordinate (y < LCD_WIDTH)
//! \param red The red component of the color
//! \param green The green component of the color
//! \param blue The blue component of the color
//! \return None.
//*****************************************************************************
void DRV_LCD_PutPixel(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue)
{
	LCD_PIXEL *p = LCD_Frame_Buffer;
	p[x * LCD_WIDTH + y].red = red;
	p[x * LCD_WIDTH + y].green = green;
	p[x * LCD_WIDTH + y].blue = blue;	
}

//*****************************************************************************
//! Clear screen
//!
//! This function is used to clear the screen.
//! \return None.
//*****************************************************************************
void DRV_LCD_ClrScr()
{
	uint32_t i,j;
	for (i = 0; i < LCD_HEIGHT; i++)
	{
		for (j = 0; j < LCD_WIDTH; j++)
		{
			DRV_LCD_PutPixel(i, j, 0, 0, 0);
		}
	}		
}

//*****************************************************************************
//! Draw a character mapping using a big font
//!
//! This function is used to draw a character mapping on the screen using a big font of the specified color at the desired coordinates.
//! \param Xpos x coordinate (x < LCD_HEIGHT)
//! \param Ypos y coordinate (y < LCD_WIDTH)
//! \param c A pointer to the location of the character font mapping
//! \param TextColor The color of the character
//! \param BackgroundTextColor The color of the background of the character
//! \return None.
//! \private
//*****************************************************************************
void DRV_LCD_DrawChar_Big(uint8_t Xpos, uint16_t Ypos, const uint16_t *c, LCD_PIXEL TextColor, LCD_PIXEL BackgroundTextColor) /* 16bit char */
{
 uint32_t line_index = 0, pixel_index = 0;
  uint8_t Xaddress = 0;
  uint16_t Yaddress = 0;
  __IO uint16_t tmp_color = 0;

  Xaddress = Xpos;
  Yaddress = Ypos;

  for (line_index = 0; line_index < GL_FONT_BIG_HEIGHT; line_index++)
  {
    /* SmallFonts have bytes in reverse order */
    if (((((const uint16_t*)c)[line_index] & (1 << pixel_index)) == 0x00))
        
    {
      DRV_LCD_PutPixel(Xaddress, Yaddress--, BackgroundTextColor.red, BackgroundTextColor.green, BackgroundTextColor.blue);
    }
    else
    {
      DRV_LCD_PutPixel(Xaddress, Yaddress--, TextColor.red, TextColor.green, TextColor.blue);
    }

    

    for (pixel_index = GL_FONT_BIG_WIDTH - 1; pixel_index > 0; pixel_index--)
    {
      /* SmallFonts have bytes in reverse order */
      if (((((const uint16_t*)c)[line_index] & (1 << pixel_index)) == 0x00))
          
      {
		DRV_LCD_PutPixel(Xaddress, Yaddress--, BackgroundTextColor.red, BackgroundTextColor.green, BackgroundTextColor.blue);
        
      }
      else
      {
		DRV_LCD_PutPixel(Xaddress, Yaddress--, TextColor.red, TextColor.green, TextColor.blue);
        
      }

      
    }
    pixel_index++;
    /* SmallFonts have bytes in reverse order */
    if (((((const uint16_t*)c)[line_index] & (1 << 0)) == 0x00))
        

    {
      DRV_LCD_PutPixel(Xaddress, Yaddress--, BackgroundTextColor.red, BackgroundTextColor.green, BackgroundTextColor.blue);
    }
    else
    {
      DRV_LCD_PutPixel(Xaddress, Yaddress--, TextColor.red, TextColor.green, TextColor.blue);
    }

    

    Xaddress++;
    Yaddress = Ypos;
  }
}

//*****************************************************************************
//! Draw a character mapping using a small font
//!
//! This function is used to draw a character mapping on the screen using a small font of the specified color at the desired coordinates.
//! \param Xpos x coordinate (x < LCD_HEIGHT)
//! \param Ypos y coordinate (y < LCD_WIDTH)
//! \param c A pointer to the location of the character font mapping
//! \param TextColor The color of the character
//! \param BackgroundTextColor The color of the background of the character
//! \return None.
//! \private
//*****************************************************************************
void DRV_LCD_DrawChar_Small(uint8_t Xpos, uint16_t Ypos, const uint16_t *c, LCD_PIXEL TextColor, LCD_PIXEL BackgroundTextColor) /* 16bit char */
{
 uint32_t line_index = 0, pixel_index = 0;
  uint8_t Xaddress = 0;
  uint16_t Yaddress = 0;
  __IO uint16_t tmp_color = 0;

  Xaddress = Xpos;
  Yaddress = Ypos;

  for (line_index = 0; line_index < GL_FONT_SMALL_HEIGHT; line_index++)
  {
    /* SmallFonts have bytes in reverse order */
	  
    if (((((const uint16_t*)c)[line_index] & (0x80 >> pixel_index)) == 0x00))
        
    {
      DRV_LCD_PutPixel(Xaddress, Yaddress--, BackgroundTextColor.red, BackgroundTextColor.green, BackgroundTextColor.blue);
    }
    else
    {
      DRV_LCD_PutPixel(Xaddress, Yaddress--, TextColor.red, TextColor.green, TextColor.blue);
    }

    

    for (pixel_index = GL_FONT_SMALL_WIDTH; pixel_index > 0; pixel_index--)
    {
      /* SmallFonts have bytes in reverse order */
      if (((((const uint16_t*)c)[line_index] & (0x80 >> pixel_index)) == 0x00))
          
      {
		DRV_LCD_PutPixel(Xaddress, Yaddress--, BackgroundTextColor.red, BackgroundTextColor.green, BackgroundTextColor.blue);
        
      }
      else
      {
		DRV_LCD_PutPixel(Xaddress, Yaddress--, TextColor.red, TextColor.green, TextColor.blue);
        
      }

      
    }
    pixel_index++;
    /* SmallFonts have bytes in reverse order */
    if (((((const uint16_t*)c)[line_index] & (0x80 >> 0)) == 0x00))
    {
      DRV_LCD_PutPixel(Xaddress, Yaddress--, BackgroundTextColor.red, BackgroundTextColor.green, BackgroundTextColor.blue);
    }
    else
    {
      DRV_LCD_PutPixel(Xaddress, Yaddress--, TextColor.red, TextColor.green, TextColor.blue);
    }

    

    Xaddress++;
    Yaddress = Ypos;
  }
}

//*****************************************************************************
//! Print a character using a small font
//!
//! This function is used to print a character on the screen using a small font of the specified color at the desired coordinates.
//! \param Line x coordinate (x < LCD_HEIGHT)
//! \param Column y coordinate (y < LCD_WIDTH)
//! \param Ascii The character to be printed 
//! \param TextColor The color of the character
//! \param BackgroundTextColor The color of the background of the character
//! \return None.
//*****************************************************************************
void DRV_LCD_DisplayCharSmall(uint16_t Line, uint16_t Column, uint8_t Ascii, LCD_PIXEL TextColor, LCD_PIXEL BackgroundTextColor)
{
  Ascii -= 32;
  DRV_LCD_DrawChar_Small(Line, Column, (&GL_Font8x8.table[Ascii * GL_FONT_SMALL_HEIGHT]),TextColor, BackgroundTextColor );
}

//*****************************************************************************
//! Print a character using a small font
//!
//! This function is used to print a character on the screen using a small font of the specified color at the desired coordinates.
//! \param Line x coordinate (x < LCD_HEIGHT)
//! \param Column y coordinate (y < LCD_WIDTH)
//! \param Ascii The character to be printed 
//! \param TextColor The color of the character
//! \param BackgroundTextColor The color of the background of the character
//! \return None.
//*****************************************************************************
void DRV_LCD_DisplayChar(uint16_t Line, uint16_t Column, uint8_t Ascii, LCD_PIXEL TextColor, LCD_PIXEL BackgroundTextColor)
{
  Ascii -= 32;
  DRV_LCD_DrawChar_Big(Line, Column, (&GL_Font16x24.table[Ascii * GL_FONT_BIG_HEIGHT]), TextColor, BackgroundTextColor);

}

//*****************************************************************************
//! Print a zero terminated string using a small font
//!
//! This function is used to print a zero terminated string on the screen using a small font of the specified color at the desired coordinates.
//! \param string The zero terminated string that will be printed
//! \param x x coordinate (x < LCD_HEIGHT)
//! \param y y coordinate (y < LCD_WIDTH)
//! \param TextColor The color of the characters of the string
//! \param BackgroundTextColor The color of the background of the characters of the string
//! \return None.
//*****************************************************************************
void DRV_LCD_Puts_Small(char *string, uint16_t x, uint16_t y, LCD_PIXEL TextColor, LCD_PIXEL BackgroundTextColor)
{
	uint32_t i = 0;
	uint16_t currentX = x;
	i = 0;
	while (string[i] != 0)
	{
		DRV_LCD_DisplayCharSmall(y, currentX, string[i], TextColor, BackgroundTextColor);
		i++;
		currentX += (GL_FONT_SMALL_WIDTH);
	}
}

//*****************************************************************************
//! Print a zero terminated string using a big font
//!
//! This function is used to print a zero terminated string on the screen using a big font of the specified color at the desired coordinates.
//! \param string The zero terminated string that will be printed
//! \param x x coordinate (x < LCD_HEIGHT)
//! \param y y coordinate (y < LCD_WIDTH)
//! \param TextColor The color of the characters of the string
//! \param BackgroundTextColor The color of the background of the characters of the string
//! \return None.
//*****************************************************************************
void DRV_LCD_Puts_Big(char *string, uint16_t x, uint16_t y, LCD_PIXEL TextColor, LCD_PIXEL BackgroundTextColor)
{
	uint32_t i = 0;
	uint16_t currentX = x;
	i = 0;
	while (string[i] != 0)
	{
		DRV_LCD_DisplayChar(y, currentX, string[i], TextColor, BackgroundTextColor);
		i++;
		currentX += (GL_FONT_BIG_WIDTH);
	}
}

//*****************************************************************************
//! Print a zero terminated string using a big or small font
//!
//! This function is used to print a zero terminated string on the screen using a big or small font of the specified color at the desired coordinates.
//! \param string The zero terminated string that will be printed
//! \param x x coordinate (x < LCD_HEIGHT)
//! \param y y coordinate (y < LCD_WIDTH)
//! \param TextColor The color of the characters of the string
//! \param BackgroundTextColor The color of the background of the characters of the string
//! \param big A boolean specifing whether the font will be small if parameter value is \b false or big if the parameter value is \b true 
//! \return None.
//*****************************************************************************
void DRV_LCD_Puts(char *string, uint16_t x, uint16_t y, LCD_PIXEL TextColor, LCD_PIXEL BackgroundTextColor, BOOLEAN big)
{
	if (big == FALSE)
	{
		DRV_LCD_Puts_Small(string, x, y, TextColor, BackgroundTextColor);	
	}
	else
	{
		DRV_LCD_Puts_Big(string, x, y, TextColor, BackgroundTextColor);	
	}	
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
