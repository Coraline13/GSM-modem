//*****************************************************************************
//! \file	drv_sdram.c
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	SDRAM Driver
//! 
//! Contains the driver for the SDRAM memory on the Olimex board
//! The SDRAM Memory can be used after the call of DRV_SDRAM_Init and after this call
//! the memory becomes transparent and may be access started from address defined by SDRAM_BASE_ADDR
//! and the size defined by SDRAM_SIZE
//*****************************************************************************

//*****************************************************************************
//! \addtogroup DRV_SDRAM
//! @{
//! \brief  Driver for the SDRAM memory on the Olimex board.
//! 
//! The SDRAM Memory can be used after the call of DRV_SDRAM_Init and after this call
//! the memory becomes transparent and may be access started from address defined by SDRAM_BASE_ADDR
//! and the size defined by SDRAM_SIZE
//*****************************************************************************

//*****************************************************************************
//! \headerfile drv_sdram.h "drv_sdram.h"
//*****************************************************************************


#include "drv_sdram.h"
#include <stdint.h>
#include "LPC177x_8x.h"
#include "system_LPC177x_8x.h"


//*****************************************************************************
//! Calculates the SDRAM period based on EMCClock global variable.
//! \return SDRAM Period.
//! \private
//*****************************************************************************
float DRV_SDRAM_PERIOD(void)
{
	return ((double)1000000000 / EMCClock);
}

//*****************************************************************************
//! Converts from time period to SDRAM clock cycles. The period is taken from DRV_SDRAM_PERIOD
//! \param period represents the value of the period to be converted
//! \return The value of the period in cc based on DRV_SDRAM_PERIOD
//! \private
//*****************************************************************************
uint32_t DRV_SDRAM_P2C(uint32_t period)
{
	float sdram_period = DRV_SDRAM_PERIOD();
	float result = 0;
	if (period < sdram_period) 
	{
		return 0;
	}
	else
	{
		result = (float)period / sdram_period;
		return (uint32_t)result;
	}
	
}

//*****************************************************************************
//! Initializes the EMC - External Memory Controller of the LPC1788 microcontroller
//! \return None.
//! \private
//*****************************************************************************
void DRV_SDRAM_InitPins()
{
	volatile uint32_t i = 0;
	volatile uint32_t temp = 0;
	volatile unsigned long Dummy = 0;
	// enable clock to EMC
	LPC_SC->PCONP |= 1 << 11; // PCEMC = 1
	
	LPC_IOCON->P2_16 = 0x21;
	LPC_IOCON->P2_17 = 0x21;
	LPC_IOCON->P2_18 = 0x21;
	LPC_IOCON->P2_20 = 0x21;
	LPC_IOCON->P2_24 = 0x21;
	LPC_IOCON->P2_28 = 0x21;
	LPC_IOCON->P2_29 = 0x21;

	LPC_IOCON->P3_0 = 0x21;
	LPC_IOCON->P3_1 = 0x21;
	LPC_IOCON->P3_2 = 0x21;
	LPC_IOCON->P3_3 = 0x21;
	LPC_IOCON->P3_4 = 0x21;
	LPC_IOCON->P3_5 = 0x21;
	LPC_IOCON->P3_6 = 0x21;
	LPC_IOCON->P3_7 = 0x21;
	LPC_IOCON->P3_8 = 0x21;
	LPC_IOCON->P3_9 = 0x21;
	LPC_IOCON->P3_10 = 0x21;
	LPC_IOCON->P3_11 = 0x21;
	LPC_IOCON->P3_12 = 0x21;
	LPC_IOCON->P3_13 = 0x21;
	LPC_IOCON->P3_14 = 0x21;
	LPC_IOCON->P3_15 = 0x21;

	LPC_IOCON->P4_0 = 0x21;
	LPC_IOCON->P4_1 = 0x21;
	LPC_IOCON->P4_2 = 0x21;
	LPC_IOCON->P4_3 = 0x21;
	LPC_IOCON->P4_4 = 0x21;
	LPC_IOCON->P4_5 = 0x21;
	LPC_IOCON->P4_6 = 0x21;
	LPC_IOCON->P4_7 = 0x21;
	LPC_IOCON->P4_8 = 0x21;
	LPC_IOCON->P4_9 = 0x21;
	LPC_IOCON->P4_10 = 0x21;
	LPC_IOCON->P4_11 = 0x21;
	LPC_IOCON->P4_12 = 0x21;
	LPC_IOCON->P4_13 = 0x21;
	LPC_IOCON->P4_14 = 0x21;
	LPC_IOCON->P4_25 = 0x21;
	
	/*Init SDRAM controller*/
	LPC_SC->EMCDLYCTL = 0x08;
	/*Set data read delay*/
	LPC_SC->EMCDLYCTL |= 0x08 << 8;
	LPC_SC->EMCDLYCTL |= 0x08 << 16;
	// enable emc
	LPC_EMC->Control = 1;
	LPC_EMC->DynamicReadConfig = 1;
	
	LPC_EMC->DynamicRasCas0 = 3 | (3 << 8);
	LPC_EMC->DynamicRP = DRV_SDRAM_P2C(SDRAM_TRP);
	LPC_EMC->DynamicRAS = DRV_SDRAM_P2C(SDRAM_TRAS);
	LPC_EMC->DynamicSREX = DRV_SDRAM_P2C(SDRAM_TXSR);
	LPC_EMC->DynamicAPR = SDRAM_TAPR;
	LPC_EMC->DynamicDAL = SDRAM_TDAL+DRV_SDRAM_P2C(SDRAM_TRP);
	LPC_EMC->DynamicWR = SDRAM_TWR;
	LPC_EMC->DynamicRC = DRV_SDRAM_P2C(SDRAM_TRC);
	LPC_EMC->DynamicRFC = DRV_SDRAM_P2C(SDRAM_TRFC);
	LPC_EMC->DynamicXSR = DRV_SDRAM_P2C(SDRAM_TXSR);
	LPC_EMC->DynamicRRD = DRV_SDRAM_P2C(SDRAM_TRRD);
	LPC_EMC->DynamicMRD = SDRAM_TMRD;
	
	LPC_EMC->DynamicConfig0 = 0x0000680;
	// JEDEC General SDRAM Initialization Sequence
	// DELAY to allow power and clocks to stabilize ~100 us
	// NOP	
	LPC_EMC->DynamicControl = 0x0183;
	for (i = 200*30; i;i--);

// PALL
	temp = LPC_EMC->DynamicControl;
	temp &= ~(3 << 7);
	temp |= 2 << 7;
	LPC_EMC->DynamicControl = temp;
	
	LPC_EMC->DynamicRefresh = 1;
	for (i = 256; i; --i);
	LPC_EMC->DynamicRefresh = DRV_SDRAM_P2C(SDRAM_REFRESH) >> 4;

	// COMM
	temp = LPC_EMC->DynamicControl;
	temp &= ~(3 << 7);
	temp |= 1 << 7;
	LPC_EMC->DynamicControl = temp;

	// Burst 4, Sequential, CAS-3
	Dummy = *(volatile unsigned short *)((uint32_t*)(SDRAM_BASE_ADDR + (0x33UL << (12))));
	
	// NORM
	
	LPC_EMC->DynamicControl = 0x0000;
	LPC_EMC->DynamicConfig0 |= 1 << 19;
	
	for (i = 100000; i; i--);
}

//*****************************************************************************
//
//! Initialize the SDRAM Controller
//!
//! This function is used to initialize the EMC controller of the processor to use the SDRAM memory on the board
//! \return None.
//
//! \public
//*****************************************************************************
void DRV_SDRAM_Init()
{
	DRV_SDRAM_InitPins();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

