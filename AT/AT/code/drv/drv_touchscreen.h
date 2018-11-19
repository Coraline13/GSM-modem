//*****************************************************************************
//! \file	drv_touchscreen.h
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	TOUCH SCREEN Driver
//! 
//! 
//! Contains the header file of the driver for the touchscreen of the Olimex board
//*****************************************************************************
#ifndef __DRV_TOUCHSCREEN_H
#define __DRV_TOUCHSCREEN_H

#include <stdint.h>
#include "drv_general.h"
//*****************************************************************************
//! \addtogroup DRV_TOUCHSCREEN
//! @{
//*****************************************************************************




//*****************************************************************************
//! \struct TouchResult
//! The structure used for encapsulating the result of a touch operation. It offers the X and Y coordinates of the detected touch
//! 
//*****************************************************************************
typedef struct
{
	uint16_t  X;	/*!< X coordinate of the detected touch*/
	uint16_t  Y;	/*!< Y coordinate of the detected touch*/
} TouchResult;

typedef void (*TOUCHSCREEN_CallBack)(TouchResult* touchData);




void DRV_TOUCHSCREEN_Init(void);
BOOLEAN TouchGet (TouchResult * pData);
STATUS DRV_TOUCHSCREEN_SetTouchCallback(TOUCHSCREEN_CallBack callback);
void DRV_TOUCHSCREEN_Process(void);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#endif
