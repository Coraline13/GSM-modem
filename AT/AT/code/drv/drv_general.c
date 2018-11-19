//*****************************************************************************
//! \file	drv_general.c
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	General structures, types, functions and defines
//! 
//! Contains various types, functions and defines used by the rest of this driver library
//*****************************************************************************

//*****************************************************************************
//! \headerfile drv_general.h "drv_general.h"
//*****************************************************************************

//*****************************************************************************
//! \addtogroup DRV_GENERAL
//! @{
//! \brief	General structures, types, functions and defines
//! 
//! Contains various types, functions and defines used by the rest of this driver library
//*****************************************************************************

#include "drv_general.h"

#include <stdint.h>
#include "LPC177x_8x.h"
#include "system_LPC177x_8x.h"


//*****************************************************************************
//! Calculates the first digit of a number
//!
//! This function is used to extract the first digit of a number
//! \param n The number to extract the 1st digit from
//! \return The 1st digit of the number
//! \private
//*****************************************************************************
uint8_t DRV_GENERAL_GetFirstDigit(uint32_t n)
{
	uint32_t temp = n;
	while (temp != 0)
	{
		if ((temp / 10) == 0)
		{
			return (temp % 10);
		}
		temp = temp / 10;
	}
	return 0;
}

//*****************************************************************************
//! Calculate the baud rate divisors using the DLD method
//!
//! This function is used to calculate de divisors for a specific baudrate and peripheral input clock
//! \param PeripheralClock The Peripheral input clock in Hz
//! \param baud Desired baud rate
//! \param sampling Sampling rate (8, 16)
//! \return A structure containing DLM, DLL and DLD
//! \private
//*****************************************************************************
UART_BAUDRATE_VALUE_DLD DRV_GENERAL_CalculateBaudrateDLD(uint32_t PeripheralClock, uint32_t baud, uint8_t sampling)
{
	uint64_t temp64 = 0;
	uint32_t temp32 = 0;
	uint32_t temp;
	UART_BAUDRATE_VALUE_DLD result = {0, 0, 0};
	if ((PeripheralClock % (baud * sampling)) == 0)
	{
		// nu am nevoie de divizor fractional		
		temp32 = PeripheralClock / (baud * sampling);
		result.DLD = 0;
		result.DLM = (temp32 >> 8) & 0xFF;
		result.DLL = temp32 & 0xFF;
	}
	else
	{
		// am nevoie si de divizor fractional
		temp64 = ((uint64_t)PeripheralClock * 1000) / (baud * sampling);
		temp32 = temp64 - ((PeripheralClock / (baud * sampling)) * 1000);
		temp = PeripheralClock / (baud * sampling);
		result.DLM = (temp >> 8) & 0xFF;
		result.DLL = temp & 0xFF;
		temp = 16 * temp32;
		result.DLD = temp / 1000;
		temp = temp - ((temp / 1000) * 1000);
		if ((temp > 100) && (DRV_GENERAL_GetFirstDigit(temp) >= 5))
		{
			result.DLD++;
		}		
	}
	return result;
}

//*****************************************************************************
//! Calculate the baud rate divisors using the fractional divisor as mval/dval
//!
//! This function is used to calculate de divisors for a specific baudrate and peripheral input clock
//! \param PeripheralClock The Peripheral input clock in Hz
//! \param baudrate Desired baud rate
//! \param sampling Sampling rate (8, 16)
//! \return A structure containing DLM, DLL and the fractional components mval and dval
//! \private
//*****************************************************************************
UART_BAUDRATE_VALUE_DIV_MUL	 DRV_GENERAL_CalculateBaudrateDivMul(uint32_t PeripheralClock, uint32_t baudrate, uint8_t sampling)
{
	UART_BAUDRATE_VALUE_DIV_MUL result = {0,0,0,0};
	uint32_t uClk;
	uint32_t rate16 = 16 * baudrate;
	uint32_t dval, mval;
	uint32_t dl;
	uClk = PeripheralClock;
  // The fractional is calculated as
   // (PCLK  % (16 * Baudrate)) / (16 * Baudrate)
   // Let's make it to be the ratio
   // DivVal / MulVal
   //

   dval = uClk % rate16;
 
   // The PCLK / (16 * Baudrate) is fractional
   // => dval = pclk % rate16;
   // mval = rate16;
   // now mormalize the ratio
   // dval / mval = 1 / new_mval;
   // new_mval = mval / dval
   // new_dval = 1
   if (dval > 0)
   {
      mval = rate16 / dval;
      dval = 1;
 
      // in case mval still bigger then 4 bits
      // no adjustment require
      if (mval > 12)
      {
         dval = 0;
      }
   }
   dval &= 0xf;
   mval &= 0xf;
 
   dl = uClk / (rate16);
   
	result.DLM = (dl >> 8) & 0xFF;
   result.DLL =  dl & 0xFF;
//   result.dval = dval;
  // result.mval = mval;


	return result;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
