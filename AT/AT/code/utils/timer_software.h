//*****************************************************************************
//! \file	timer_software.h
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	Timer software library
//! 
//! Contains a library that implements a software timer module
//*****************************************************************************

#ifndef __TIMER_SOFTWARE_H
#define __TIMER_SOFTWARE_H

//*****************************************************************************
//! \addtogroup TimerSoftware
//! @{
//*****************************************************************************
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SW_TIMER_PERIOD 	1000 // (us)  /**< Defines the software timer tick in microseconds  */
#define MAX_NR_TIMERS 		16			  /**< Maximum available timers  */

//*****************************************************************************
//! \enum SOFTWARE_TIMER_MODE
//! Defines the software timers possible operating modes
//*****************************************************************************
typedef enum 
{
	MODE_0,
	MODE_1,
	MODE_2,
	MODE_3
}SOFTWARE_TIMER_MODE;

//*****************************************************************************
//! \typedef timer_software_handler_t
//! Defines the software timer handler type
//
//*****************************************************************************
typedef  int16_t timer_software_handler_t;

//*****************************************************************************
//! \typedef TIMER_SOFTWARE_Callback
//! Defines the callback function type
//
//*****************************************************************************
typedef void (*TIMER_SOFTWARE_Callback)(timer_software_handler_t);


//*****************************************************************************
//! \struct SOFTWARE_TIMER
//! The structure defines the registers for a software timer
//
//*****************************************************************************
typedef struct 
{
	/* Timer Control
	Bit 0 - Timer Valid (1), Timer Invalid (0)
	Bit 1 - Timer Enable (1), Timer Disable (0)
	Bit 3,2 - Timer Mode
			 0 0 - Count to period and stop
			 0 1 - Count to period with reset and restart
			 1 0 - Count to period, change the flag, and keep counting
			 1 1 - Free run
	*/
	volatile uint8_t TimerControl;											/*!< Software timer control register*/
	volatile uint32_t TimerPeriod; 											/*!< Software timer period*/
	volatile uint32_t TimerCounter; // incremented every modx execution		/*!< Software timer counter register*/
	/*
		Timer Status Register
		Bit 0 Running Flag - Timer Running(1), Timer Stopped (0)
		Bit 1 Error Flag - Error(1), NoError(0)
		Bit 2 InterruptFlag - Interrupt Pending (1), No Interrupt pending (0)
		Bit 3 Overflow Flag - Timer overflow (1), Timer did not overflow (0)
	*/
	volatile uint8_t TimerStatus;											/*!< Software timer status register*/
	TIMER_SOFTWARE_Callback callback;										/*!< Software timer callback address register*/
}SOFTWARE_TIMER;

//extern volatile SOFTWARE_TIMER timers[];

void TIMER_SOFTWARE_ModX(void);
void TIMER_SOFTWARE_init(void);
uint8_t TIMER_SOFTWARE_release_timer(timer_software_handler_t timer_handler);
timer_software_handler_t TIMER_SOFTWARE_request_timer(void);
int8_t TIMER_SOFTWARE_configure_timer(timer_software_handler_t timer_handler, SOFTWARE_TIMER_MODE timer_mode, uint32_t period, uint8_t enable);
int8_t TIMER_SOFTWARE_enable_timer(timer_software_handler_t timer_handler);
int8_t TIMER_SOFTWARE_disable_timer(timer_software_handler_t timer_handler);
int8_t TIMER_SOFTWARE_start_timer(timer_software_handler_t timer_handler);
int8_t TIMER_SOFTWARE_stop_timer(timer_software_handler_t timer_handler);
int8_t TIMER_SOFTWARE_set_callback(timer_software_handler_t timer_handler, TIMER_SOFTWARE_Callback callback);
void TIMER_SOFTWARE_Wait(uint32_t time);
void TIMER_SOFTWARE_reset_timer(timer_software_handler_t timer_handler);
uint8_t TIMER_SOFTWARE_interrupt_pending(timer_software_handler_t timer_handler);
void TIMER_SOFTWARE_clear_interrupt(timer_software_handler_t timer_handler);
#ifdef __cplusplus
}
#endif
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#endif
