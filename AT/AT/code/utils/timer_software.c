//*****************************************************************************
//! \file	timer_software.c
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	Timer software library
//! 
//! Contains a library that implements a software timer module
//*****************************************************************************

//*****************************************************************************
//! \headerfile timer_software.h "timer_software.h"
//*****************************************************************************

//*****************************************************************************
//! \addtogroup TimerSoftware
//! @{
//! \brief	Timer software library
//! 
//! Contains a library that implements a software timer module
//*****************************************************************************

#include <stdint.h>
#include "timer_software.h"

//*****************************************************************************
/*! \var SOFTWARE_TIMER timers[MAX_NR_TIMERS];
	\brief The software timers structures. 
*/
//*****************************************************************************
volatile SOFTWARE_TIMER timers[MAX_NR_TIMERS];

//*****************************************************************************
/*! \var timer_software_handler_t wait_timer
	\brief Defines a software timer needed for a the function. 
*/
//*****************************************************************************
timer_software_handler_t wait_timer;

#define VALIDATE_TIMER(timer_id) 				(timers[timer_id].TimerControl |= 1)
#define INVALIDATE_TIMER(timer_id)				(timers[timer_id].TimerControl &= ~1)
#define TIMER_IS_VALID(timer_id)				( (timers[timer_id].TimerControl & 1) ? 1 : 0)

#define TIMER_ENABLE(timer_id)  				(timers[timer_id].TimerControl |= 2)
#define TIMER_DISABLE(timer_id) 				(timers[timer_id].TimerControl &= ~2)
#define TIMER_IS_ENABLED(timer_id)				( (timers[timer_id].TimerControl & 2) ? 1 : 0) 

#define TIMER_SET_MODE_0(timer_id)				(timers[timer_id].TimerControl &= ~0x0C)
#define TIMER_SET_MODE_1(timer_id)				(timers[timer_id].TimerControl = ((timers[timer_id].TimerControl & (~(1 << 3))) | (1 << 2)))
#define TIMER_SET_MODE_2(timer_id)				(timers[timer_id].TimerControl = ((timers[timer_id].TimerControl & (~(1 << 2))) | (1 << 3)))  
#define TIMER_SET_MODE_3(timer_id)				(timers[timer_id].TimerControl |= ((1 << 2) | (1 << 3)))
#define TIMER_GET_MODE(timer_id)				((timers[timer_id].TimerControl >> 2) & 0x03)


#define TIMER_SET_PERIOD(timer_id, period)		(timers[ timer_id ].TimerPeriod = period)
#define TIMER_GET_PERIOD(timer_id)				(timers[ timer_id ].TimerPeriod)

#define TIMER_GET_COUNTER(timer_id)				(timers[ timer_id ].TimerCounter)
#define TIMER_SET_COUNTER(timer_id, counter)	(timers[ timer_id ].TimerCounter = counter)
#define TIMER_RESET(timer_id)					(timers[ timer_id ].TimerCounter = 0)

#define TIMER_SET_RUNNING_FLAG(timer_id)		(timers[timer_id].TimerStatus |= 1)
#define TIMER_CLR_RUNNING_FLAG(timer_id)		(timers[timer_id].TimerStatus &= ~1)
#define TIMER_IS_RUNNING(timer_id)				( (timers[timer_id].TimerStatus & 1) ? 1 : 0)

#define TIMER_SET_ERROR_FLAG(timer_id)			(timers[timer_id].TimerStatus |= (1 << 1))
#define TIMER_CLR_ERROR_FLAG(timer_id)			(timers[timer_id].TimerStatus &= ~(1 << 1))
#define TIMER_IS_IN_ERROR_STATE(timer_id)		( (timers[timer_id].TimerStatus & (1 << 1)) ? 1 : 0)

#define TIMER_SET_INTERRUPT_FLAG(timer_id)		(timers[timer_id].TimerStatus |= (1 << 2))
#define TIMER_CLR_INTERRUPT_FLAG(timer_id)		(timers[timer_id].TimerStatus &= ~(1 << 2))
#define TIMER_INTERRUPT_PENDING(timer_id)		( (timers[timer_id].TimerStatus & (1 << 2)) ? 1 : 0)

#define TIMER_SET_OVERFLOW_FLAG(timer_id)		(timers[timer_id].TimerStatus |= (1 << 3))
#define TIMER_CLR_OVERFLOW_FLAG(timer_id)		(timers[timer_id].TimerStatus &= ~(1 << 3))
#define TIMER_IS_OVERFLOW(timer_id)				( (timers[timer_id].TimerStatus & (1 << 3)) ? 1 : 0)


//*****************************************************************************
//! The software timer internal processing function. This is called at a period of 1 ms by a hardware timer
//! 
//! \private
//*****************************************************************************
void TIMER_SOFTWARE_ModX()
{
	uint8_t i;
	for (i = 0; i < MAX_NR_TIMERS; i++)
	{
		if (TIMER_IS_VALID(i))
		{
			if (TIMER_IS_ENABLED(i) && !TIMER_IS_IN_ERROR_STATE(i))
			{
				if (TIMER_IS_RUNNING(i))
				{
					timers[i].TimerCounter++;
					if (TIMER_GET_COUNTER(i) == 0xFFFFFFFF)
					{
						TIMER_SET_OVERFLOW_FLAG(i);
					}
					switch (TIMER_GET_MODE(i))
					{
						case MODE_0:
						{			
							if (TIMER_GET_COUNTER(i) >= TIMER_GET_PERIOD(i))
							{
								TIMER_SET_INTERRUPT_FLAG(i);
								TIMER_CLR_RUNNING_FLAG(i);
								TIMER_RESET(i);
								if (timers[i].callback != 0)
								{
									TIMER_CLR_INTERRUPT_FLAG(i);
									(timers[i].callback)(i);
								}
							}
							break;
						}
						case MODE_1:
						{
							if (TIMER_GET_COUNTER(i) == TIMER_GET_PERIOD(i))
							{
								TIMER_SET_INTERRUPT_FLAG(i);
								TIMER_RESET(i);
								if (timers[i].callback != 0)
								{
									TIMER_CLR_INTERRUPT_FLAG(i);
									(timers[i].callback)(i);
								}
							}
							break;
						}
						case MODE_2:
						{
							if (TIMER_GET_COUNTER(i) == TIMER_GET_PERIOD(i))
							{
								TIMER_SET_INTERRUPT_FLAG(i);
								if (timers[i].callback != 0)
								{
									TIMER_CLR_INTERRUPT_FLAG(i);
									(timers[i].callback)(i);
								}
								
							}									
							break;
						}							
						case MODE_3:
						{
							// free run
							break;
						}
					}
				}
			}			
		}		
	}
}


//*****************************************************************************
//! Initializes the software timer structure. This function is called by \ref TIMER_SOFTWARE_init_system
//! 
//! \private
//*****************************************************************************
void TIMER_SOFTWARE_init()
{
	uint8_t i;
	for (i = 0; i < MAX_NR_TIMERS; i++)
	{
		timers[i].TimerControl = 0;
		timers[i].TimerPeriod = 0;
		timers[i].TimerCounter = 0;
		timers[i].TimerStatus = 0;
		timers[i].callback = 0;
		TIMER_SET_ERROR_FLAG(i);
	}
	wait_timer = TIMER_SOFTWARE_request_timer();
}

//*****************************************************************************
//! Release a previously used software timer
//! 
//! \param timer_handler The handler of the software timer
//! \return \b 1 for error 
//! \return \b 0 for success
//*****************************************************************************
uint8_t TIMER_SOFTWARE_release_timer(timer_software_handler_t timer_handler)
{
	if (timer_handler >= MAX_NR_TIMERS)
	{
		return 1;
	}
	timers[timer_handler].TimerControl = 0;
	timers[timer_handler].TimerPeriod = 0;
	timers[timer_handler].TimerCounter = 0;
	timers[timer_handler].TimerStatus = 0;
	TIMER_SET_ERROR_FLAG(timer_handler);
	return 0;
}

//*****************************************************************************
//! Request a new software timer. The returned value is a handler associated to the requested software timer. All operations of the requested timer will require this handler
//! 
//! \return The handler of the software timer
//*****************************************************************************
timer_software_handler_t TIMER_SOFTWARE_request_timer()
{
	uint8_t i;
	uint8_t found = 0;
	// find the first available timer	
	for (i = 0; i < MAX_NR_TIMERS; i++)
	{
		if (!TIMER_IS_VALID(i))
		{
			found = 1;
			break;
		}
	}
	if (found)
	{
		timers[i].TimerControl = 0;
		timers[i].TimerPeriod = 0;
		timers[i].TimerCounter = 0;
		timers[i].TimerStatus = 0;
		TIMER_SET_ERROR_FLAG(i);
		VALIDATE_TIMER(i);
		return i;
	}
	return -1;
}
//*****************************************************************************
//! Configure a software timer
//! 
//! \param timer_handler The handler of the software timer to configure. The handler needs to be obtained with \ref TIMER_SOFTWARE_request_timer
//! \param timer_mode The operating mode of the software timer. See \ref SOFTWARE_TIMER_MODE
//! \param period The period of the software timer
//! \param enable Designates if the software timer should be automatically enabled (not started) after configuration
//! \return \b -1 for error 
//! \return \b 0 for success
//*****************************************************************************
int8_t TIMER_SOFTWARE_configure_timer(timer_software_handler_t timer_handler, SOFTWARE_TIMER_MODE timer_mode, uint32_t period, uint8_t enable)
{
	if (!TIMER_IS_VALID(timer_handler))
	{
		return -1;
	}

	TIMER_CLR_ERROR_FLAG(timer_handler);
	switch (timer_mode)
	{
		case MODE_0:
		{
			TIMER_SET_MODE_0(timer_handler);
			if (period < 2)
			{
				TIMER_SET_ERROR_FLAG(timer_handler);				
			}
			TIMER_SET_PERIOD(timer_handler, period);
			break;
		}
		case MODE_1:
		{
			TIMER_SET_MODE_1(timer_handler);
			if (period < 2)
			{
				TIMER_SET_ERROR_FLAG(timer_handler);				
			}			
			TIMER_SET_PERIOD(timer_handler, period);
			break;
		}
		case MODE_2:
		{
			TIMER_SET_MODE_2(timer_handler);
			if (period < 2)
			{
				TIMER_SET_ERROR_FLAG(timer_handler);				
			}			
			TIMER_SET_PERIOD(timer_handler, period);
			break;
		}
		case MODE_3:
		{
			TIMER_SET_MODE_3(timer_handler);
			TIMER_SET_PERIOD(timer_handler, 0);
			break;
		}
		default:
		{
			TIMER_SET_MODE_3(timer_handler);
			TIMER_SET_ERROR_FLAG(timer_handler);
		}
	}
	if (TIMER_IS_IN_ERROR_STATE(timer_handler))
	{
		return -1;
	}

	if (enable)
	{
		TIMER_ENABLE(timer_handler);
	}
	return 0;																			  
}

//*****************************************************************************
//! Enables a software timer
//! 
//! \param timer_handler The handler of the software timer. The handler needs to be obtained with \ref TIMER_SOFTWARE_request_timer
//! \return \b -1 for error 
//! \return \b 0 for success
//*****************************************************************************
int8_t TIMER_SOFTWARE_enable_timer(timer_software_handler_t timer_handler)
{
	if (timer_handler >	MAX_NR_TIMERS)
	{
		return -1;
	}
	if (!TIMER_IS_VALID(timer_handler))
	{
		return -1;
	}
	if (TIMER_IS_IN_ERROR_STATE(timer_handler))
	{
		return -1;
	}
	TIMER_ENABLE(timer_handler);
	return 0;
}

//*****************************************************************************
//! Disables a software timer
//! 
//! \param timer_handler The handler of the software timer. The handler needs to be obtained with \ref TIMER_SOFTWARE_request_timer
//! \return \b -1 for error 
//! \return \b 0 for success
//*****************************************************************************
int8_t TIMER_SOFTWARE_disable_timer(timer_software_handler_t timer_handler)
{
	if (timer_handler >	MAX_NR_TIMERS)
	{
		return -1;
	}
	if (!TIMER_IS_VALID(timer_handler))
	{
		return -1;
	}
	TIMER_DISABLE(timer_handler);
	return 0;
}

//*****************************************************************************
//! Starts a software timer
//! 
//! \param timer_handler The handler of the software timer. The handler needs to be obtained with \ref TIMER_SOFTWARE_request_timer
//! \return \b -1 for error 
//! \return \b 0 for success
//*****************************************************************************
int8_t TIMER_SOFTWARE_start_timer(timer_software_handler_t timer_handler)
{
	if (timer_handler >	MAX_NR_TIMERS)
	{
		return -1;
	}
	if (!TIMER_IS_VALID(timer_handler))
	{
		return -1;
	}
	TIMER_SOFTWARE_enable_timer(timer_handler);
	if (!TIMER_IS_ENABLED(timer_handler))
	{
		return -1;
	}
	TIMER_SET_RUNNING_FLAG(timer_handler);
	return 0;
}

//*****************************************************************************
//! Stops a software timer
//! 
//! \param timer_handler The handler of the software timer. The handler needs to be obtained with \ref TIMER_SOFTWARE_request_timer
//! \return \b -1 for error 
//! \return \b 0 for success
//*****************************************************************************
int8_t TIMER_SOFTWARE_stop_timer(timer_software_handler_t timer_handler)
{
	if (timer_handler >	MAX_NR_TIMERS)
	{
		return -1;
	}
	if (!TIMER_IS_VALID(timer_handler))
	{
		return -1;
	}
	TIMER_CLR_RUNNING_FLAG(timer_handler);
	return 0;
}

//*****************************************************************************
//! Sets the callback function of the coresponding software timer. This function will be called when a software timer expires
//! 
//! \param timer_handler The handler of the software timer. The handler needs to be obtained with \ref TIMER_SOFTWARE_request_timer
//! \param callback The pointer to the user function callback
//! \return \b -1 for error 
//! \return \b 0 for success
//*****************************************************************************
int8_t TIMER_SOFTWARE_set_callback(timer_software_handler_t timer_handler, TIMER_SOFTWARE_Callback callback)
{
	if (timer_handler >	MAX_NR_TIMERS)
	{
		return -1;
	}
	if (!TIMER_IS_VALID(timer_handler))
	{
		return -1;
	}
	timers[timer_handler].callback = callback;
	return 0;
}

//*****************************************************************************
//! A wait function that freezes execution for an amount of time. This function may be used separately of the whole driver. No other function calls are needed. It uses an internal software timer
//! 
//! \param time The amount of time in ms to wait
//*****************************************************************************
void TIMER_SOFTWARE_Wait(uint32_t time)
{
	TIMER_SOFTWARE_stop_timer(wait_timer);
	TIMER_CLR_INTERRUPT_FLAG(wait_timer);
	TIMER_SOFTWARE_configure_timer(wait_timer, MODE_0, time, 1);
	TIMER_RESET(wait_timer);
	TIMER_SOFTWARE_start_timer(wait_timer);
	while (!(TIMER_INTERRUPT_PENDING(wait_timer)));		
	TIMER_SOFTWARE_stop_timer(wait_timer);
	TIMER_RESET(wait_timer);
	TIMER_CLR_INTERRUPT_FLAG(wait_timer);	
}

//*****************************************************************************
//! Resets a software timer
//! 
//! \param timer_handler The handler of the software timer. The handler needs to be obtained with \ref TIMER_SOFTWARE_request_timer
//! \return \b -1 for error 
//! \return \b 0 for success
//*****************************************************************************
void TIMER_SOFTWARE_reset_timer(timer_software_handler_t timer_handler)
{
	TIMER_RESET(timer_handler);
}

//*****************************************************************************
//! Checks if an interrupt is pending for a designated software timer
//! 
//! \param timer_handler The handler of the software timer. The handler needs to be obtained with \ref TIMER_SOFTWARE_request_timer
//! \return \b 0 if no interrupt is pending 
//! \return \b >0 if an interrupt is pending
//*****************************************************************************
uint8_t TIMER_SOFTWARE_interrupt_pending(timer_software_handler_t timer_handler)
{
	return TIMER_INTERRUPT_PENDING(timer_handler);
}

//*****************************************************************************
//! Clears a pending software timer interrupt
//! 
//! \param timer_handler The handler of the software timer. The handler needs to be obtained with \ref TIMER_SOFTWARE_request_timer
//! \return \b -1 for error 
//! \return \b 0 for success
//*****************************************************************************
void TIMER_SOFTWARE_clear_interrupt(timer_software_handler_t timer_handler)
{
	TIMER_CLR_INTERRUPT_FLAG(timer_handler);
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
