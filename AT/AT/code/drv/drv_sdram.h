//*****************************************************************************
//! \file	drv_sdram.h
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	SDRAM Driver
//! 
//! 
//! Contains the header file of the driver for the SDRAM memory on the Olimex board
//! The SDRAM Memory can be used after the call of DRV_SDRAM_Init and after this call
//! the memory becomes transparent and may be access started from address defined by SDRAM_BASE_ADDR
//! and the size defined by SDRAM_SIZE
//*****************************************************************************

#ifndef __DRV_SDRAM_H
#define __DRV_SDRAM_H

#include <stdint.h>

//*****************************************************************************
//! \addtogroup DRV_SDRAM
//! @{
//*****************************************************************************


/*! 
	The starting address of the Olimex OnBoard SDRAM memory
*/
#define SDRAM_BASE_ADDR			0xA0000000

/*! 
	The size of Olimex OnBoard SDRAM memory
*/
#define SDRAM_SIZE 				0x02000000


#define SDRAM_REFRESH         7813
#define SDRAM_TRP             20
#define SDRAM_TRAS            45
#define SDRAM_TAPR            1
#define SDRAM_TDAL            3
#define SDRAM_TWR             3
#define SDRAM_TRC             65
#define SDRAM_TRFC            66
#define SDRAM_TXSR            67
#define SDRAM_TRRD            15
#define SDRAM_TMRD            3

void DRV_SDRAM_Init(void);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#endif
