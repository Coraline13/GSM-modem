//*****************************************************************************
//! \file	timer_software_init.c
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	Timer software library hardware abstraction layer
//! 
//! Contains the hardware abstraction layer for the software timer
//*****************************************************************************

//*****************************************************************************
//! \headerfile timer_software_init.h "timer_software_init.h"
//*****************************************************************************

//*****************************************************************************
//! \addtogroup TimerSoftware
//! @{
//! \brief	Timer software library
//! 
//! Contains a library that implements a software timer module
//*****************************************************************************

//*****************************************************************************
//! \addtogroup TimerSoftwareHAL
//! @{
//! \brief	Timer software library hardware abstraction layer
//! 
//! Contains the hardware abstraction layer for the software timer
//*****************************************************************************


#include "timer_software_init.h"
#include "timer_software.h"
#include <stdint.h>
#include "LPC177x_8x.h"
#include "system_LPC177x_8x.h"


//*****************************************************************************
//! LPC1788 Specific interrupt service routing for TIMER0 which executes the processing routing of the software timer \ref TIMER_SOFTWARE_ModX
//! 
//! \private
//*****************************************************************************

void TIMER0_IRQHandler()
{
	TIMER_SOFTWARE_ModX();
	LPC_TIM0->IR = 1; // reset interrupt
	
}

//*****************************************************************************
//! Software timer system initilization. This function is mandatory to be the first called function before using the rest of the library (including \ref TIMER_SOFTWARE_Wait)
//! 
//*****************************************************************************
void TIMER_SOFTWARE_init_system()
{
	TIMER_SOFTWARE_init();
	LPC_SC->PCONP |= 1 << 1;
	LPC_TIM0->MR0 = PeripheralClock / 1000;
	LPC_TIM0->MCR = 3; // interrupt on mr0 and reset on mr0;
	LPC_TIM0->TCR = 1; // enable timer, start counting
	NVIC->ISER[0] |= 2; // enable timer0 interrupt
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

