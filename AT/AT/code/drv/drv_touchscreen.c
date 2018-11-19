//*****************************************************************************
//! \file	drv_touchscreen.c
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	TOUCH SCREEN Driver
//! 
//! Contains the driver for the touchscreen of the Olimex board
//*****************************************************************************

//*****************************************************************************
//! \addtogroup DRV_TOUCHSCREEN
//! @{
//! \brief  Driver for the TOUCHSCREEN of the Olimex board.
//! 
//*****************************************************************************

//*****************************************************************************
//! \headerfile drv_touchscreen.h "drv_touchscreen.h"
//*****************************************************************************

#include "drv_touchscreen.h"
#include <stdint.h>
#include "LPC177x_8x.h"
#include "system_LPC177x_8x.h"
#include "drv_general.h"
#include "drv_lcd.h"
#include <utils\timer_software.h>

#define TS_X1_MASK          (1UL << 24)
#define TS_X1_FIO           LPC_GPIO0->PIN   //FIO0PIN
#define TS_X1_FDIR          LPC_GPIO0->DIR//FIO0DIR
#define TS_X1_FSET          LPC_GPIO0->SET// FIO0SET
#define TS_X1_FCLR          LPC_GPIO0->CLR// FIO0CLR
#define TS_X1_INTR_R        LPC_GPIOINT->IO0IntEnR//IO0INTENR
#define TS_X1_INTR_CLR      LPC_GPIOINT->IO0IntClr// IO0INTCLR
#define TS_X1_IOCON         LPC_IOCON->P0_24

#define TS_X2_MASK          (1UL << 22)
#define TS_X2_FIO           LPC_GPIO0->PIN//FIO0PIN
#define TS_X2_FDIR          LPC_GPIO0->DIR//FIO0DIR
#define TS_X2_FSET          LPC_GPIO0->SET//FIO0SET
#define TS_X2_FCLR          LPC_GPIO0->CLR//FIO0CLR
#define TS_X2_INTR_R        LPC_GPIOINT->IO0IntEnR//IO0INTENR
#define TS_X2_INTR_CLR      LPC_GPIOINT->IO0IntClr//IO0INTCLR
#define TS_X2_IOCON         LPC_IOCON->P0_22

#define TS_Y1_MASK          (1UL << 23)
#define TS_Y1_FIO           LPC_GPIO0->PIN//FIO0PIN
#define TS_Y1_FDIR          LPC_GPIO0->DIR//FIO0DIR
#define TS_Y1_FSET          LPC_GPIO0->SET//FIO0SET
#define TS_Y1_FCLR          LPC_GPIO0->CLR//FIO0CLR
#define TS_Y1_INTR_R        LPC_GPIOINT->IO0IntEnR//IO0INTENR
#define TS_Y1_INTR_CLR      LPC_GPIOINT->IO0IntClr//IO0INTCLR
#define TS_Y1_IOCON         LPC_IOCON->P0_23

#define TS_Y2_MASK          (1UL << 21)
#define TS_Y2_FIO           LPC_GPIO0->PIN//FIO0PIN
#define TS_Y2_FDIR          LPC_GPIO0->DIR//FIO0DIR
#define TS_Y2_FSET          LPC_GPIO0->SET//FIO0SET
#define TS_Y2_FCLR          LPC_GPIO0->CLR//FIO0CLR
#define TS_Y2_INTR_R        LPC_GPIOINT->IO0IntEnR//IO0INTENR
#define TS_Y2_INTR_CLR      LPC_GPIOINT->IO0IntClr//IO0INTCLR
#define TS_Y2_IOCON         LPC_IOCON->P0_21

#define TS_INTR_PRIORITY  2
#define TS_SETUP_DLY      50     // 100us
#define TS_SAMPLE_DLY     10000   // 10ms
#define TS_INIT_DLY       50000   // 50ms
#define TS_SAMPLES        32
#define IMAGE_WIDTH       C_GLCD_H_SIZE
#define IMAGE_HEIGHT      C_GLCD_V_SIZE

//*****************************************************************************
//! \enum _TouchScrState_t
//! \typedef _TouchScrState_t
//! \private
//! Defines the internal state of the touchscreen driver
//*****************************************************************************

typedef enum _TouchScrState_t
{
  TS_INTR_SETUP_DLY = 0, TS_WAIT_FOR_TOUCH,
  TS_X1_SETUP_DLY, TS_X1_MEASURE,
  TS_X2_SETUP_DLY, TS_X2_MEASURE,
  TS_Y1_SETUP_DLY, TS_Y1_MEASURE,
  TS_Y2_SETUP_DLY, TS_Y2_MEASURE,
} TouchScrState_t;



static volatile BOOLEAN Touch;
static volatile uint16_t  X,Y;
static volatile uint16_t  X_temp,Y_temp;
static volatile BOOLEAN Touch_temp;

static volatile TouchScrState_t State;
static volatile uint32_t Samples;

#define LEFT_UP_X     (50*TS_SAMPLES*2)   // 50
#define RIGHT_UP_X    (930*TS_SAMPLES*2)  // 930

#define LEFT_UP_Y     (200*TS_SAMPLES*2)  // 200
#define RIGHT_UP_Y    (800*TS_SAMPLES*2)  // 800

TouchResult touchResultData;
TOUCHSCREEN_CallBack currentTouchScreenCallback;

//*****************************************************************************
//! Initialize the TouchScreen Driver
//!
//! This function is used to initialize the TouchScreen controller and driver.
//! \return None.
//*****************************************************************************
void DRV_TOUCHSCREEN_Init()
{
	volatile uint32_t dummy;
	Touch_temp = Touch = FALSE;
	X = Y = 0;
	State = TS_INTR_SETUP_DLY;

	TS_X1_IOCON   = 0x101;   // ADC0 Ch1 disable pulls
	TS_X2_IOCON   = 0x28;   // general IO enable pull-down

	TS_Y1_IOCON   = 0x1A0;   // general IO disable pulls
	TS_Y2_IOCON   = 0x20;    // general IO disable pulls

	TS_X1_FDIR &= ~TS_X1_MASK;
	TS_X2_FDIR &= ~TS_X2_MASK;
	TS_Y1_FDIR |=  TS_Y1_MASK;
	TS_Y2_FDIR |=  TS_Y2_MASK;

	TS_Y1_FSET  =  TS_Y1_MASK;
	TS_Y2_FSET  =  TS_Y2_MASK;

	// Init Port interrupt
	TS_X2_INTR_R  &= ~TS_X2_MASK; // disable X2 rising edge interrupt
	TS_X2_INTR_CLR =  TS_X2_MASK;
	NVIC->ISER[1] |= 1 << 6;
	
	// init adc
	LPC_SC->PCONP |= 1 << 12;// PCAD = 1; enable adc clocks
	LPC_ADC->CR = 1 << 21; // PDN = 1; converter is operational
	LPC_ADC->CR &= ~(7 << 24); // start = 0
	LPC_ADC->CR &= ~0xFF;
	LPC_ADC->CR |= 1 << 1; // select ch1
	LPC_ADC->CR |= ((PeripheralClock / 500000) & 0xFF) << 8;
	LPC_ADC->CR &= ~(1 << 16);


	// clear all pending interrupts
	while ((LPC_ADC->STAT & (1 << 16)) != 0)
	{
		dummy = LPC_ADC->GDR;
	}
	
	LPC_ADC->INTEN |= 1 << 8;
	NVIC->ISER[0] |= 1 << 22;
	
	// Init delay timer
	LPC_SC->PCONP |= 1 << 2;
	LPC_TIM1->TCR = 2;
	LPC_TIM1->CTCR &= ~3;
	LPC_TIM1->MCR |= 1 << 2;
	LPC_TIM1->MCR |= 1 << 1;
	LPC_TIM1->MCR |= 1;
	LPC_TIM1->PR = (PeripheralClock / 1000000) - 1;
	LPC_TIM1->MR0 = TS_SETUP_DLY;
	LPC_TIM1->IR |= 1;
	
	NVIC->ISER[0] |= 1 << 2;	
	LPC_TIM1->TCR = 1;
	currentTouchScreenCallback = NULL;
}

//*****************************************************************************
//! Timer1 Interrupt Service Routine
//!
//! This function is used by the TouchScreen driver
//! \return None.
//! \private
//*****************************************************************************
void TIMER1_IRQHandler()
{
	LPC_TIM1->IR |= 1; // clear pending interrupt
	LPC_TIM1->TCR = 2;
	switch (State)
	{
		case TS_X1_SETUP_DLY:
		case TS_X2_SETUP_DLY:
		case TS_Y1_SETUP_DLY:
		case TS_Y2_SETUP_DLY:
		{
			++State;
			//LPC_ADC->CR &= ~(7 << 24);
			LPC_ADC->CR |= 1 << 24;
			break;
		}
		case TS_INTR_SETUP_DLY:
		{
			++State;
			TS_X2_INTR_CLR = TS_X2_MASK;
			if(0 == (TS_X2_FIO & TS_X2_MASK))
			{
			  Touch_temp = Touch = FALSE;
			  TS_X2_INTR_R |= TS_X2_MASK;
			}
			else
			{
				// Update X and Y
				if(Touch_temp)
				{
					X = X_temp;
					Y = Y_temp;
				}

				Touch = Touch_temp;

				// Y1 = 0, Y2 = 1;
				TS_Y1_FCLR = TS_Y1_MASK;
				// Disable X2 pull down
				TS_X2_IOCON &= ~(3<<3);
				// Reset sample counter
				Samples = 0;
				// Clear accumulators
				X_temp = Y_temp = 0;
				// Init setup delay
				if(Touch)
				{
					LPC_TIM1->MR0 = TS_SETUP_DLY;
				}
				else
				{
					LPC_TIM1->MR0 = TS_INIT_DLY;
				}
				State = TS_X1_SETUP_DLY;
				LPC_TIM1->TCR = 1;
			}
			break;
		}
		default:
		{
			break;
		}
	}
	NVIC->ICPR[0] |= 1 << 2;
}

//*****************************************************************************
//! GPIO Interrupt Service Routine
//!
//! This function is used by the TouchScreen driver
//! \return None.
//! \private
//*****************************************************************************
void GPIO_IRQHandler()
{
	// Disable and clear interrupt
	TS_X2_INTR_R  &= ~TS_X2_MASK;
	TS_X2_INTR_CLR =  TS_X2_MASK;
	// Init ACD measure setup delay
	if(TS_WAIT_FOR_TOUCH == State)
	{
		// Y1 = 0, Y2 = 1;
		TS_Y1_FCLR = TS_Y1_MASK;
		// Disable X2 pull down
		TS_X2_IOCON &= ~(3<<3);
		// Reset sample counter
		Samples = 0;
		// Clear accumulators
		X_temp = Y_temp = 0;
		// Init setup delay
		if(Touch)
		{
			LPC_TIM1->MR0 = TS_SETUP_DLY;
		}
		else
		{
			LPC_TIM1->MR0 = TS_INIT_DLY;
		}
		State = TS_X1_SETUP_DLY;
		LPC_TIM1->TCR = 1;
	}
	NVIC->ICPR[1] |= 1 << 6;	
}

//*****************************************************************************
//! Analog to Digital Converter Interrupt Service Routine
//!
//! This function is used by the TouchScreen driver
//! \return None.
//! \private
//*****************************************************************************
void ADC_IRQHandler()
{
	volatile uint32_t Data;
	LPC_ADC->CR &= ~(7 << 24);
	Data = ((LPC_ADC->GDR >> 4) & 0xFFF) >> 2;
	switch(State)
	{
		case TS_X1_MEASURE:
		{
			Y_temp += Data;
			if(++Samples >= TS_SAMPLES)
			{
				Samples = 0;
				State = TS_X2_SETUP_DLY;
				// Y2 = 0, Y1 = 1;
				TS_Y2_FCLR = TS_Y2_MASK;
				TS_Y1_FSET = TS_Y1_MASK;
				// Init setup delay
				LPC_TIM1->MR0 = TS_SETUP_DLY;
				LPC_TIM1->TCR = 1;
			}
			else
			{
				LPC_ADC->CR |= 1 << 24;
			}
			break;
		}    
		case TS_X2_MEASURE:
		{
			Y_temp += 1023UL - Data;
			if(++Samples >= TS_SAMPLES)
			{
				Samples = 0;
				State = TS_Y1_SETUP_DLY;

				// X1 = 0, X2 = 1;
				TS_X1_FCLR  = TS_X1_MASK;
				TS_X2_FSET  = TS_X2_MASK;
				TS_X1_FDIR |= TS_X1_MASK;
				TS_X2_FDIR |= TS_X2_MASK;
				TS_X1_IOCON   = 0x1A0;

				// Y1 - ADC Ch0, Y2 input
				TS_Y1_FDIR &= ~TS_Y1_MASK;
				TS_Y2_FDIR &= ~TS_Y2_MASK;
				TS_Y1_IOCON   = 0x101;            // assign to ADC0 Ch0
				LPC_ADC->CR &= ~0xFF;
				LPC_ADC->CR |= 1UL<<0;    // select Ch0

				// Init setup delay
				LPC_TIM1->MR0 = TS_SETUP_DLY;
				LPC_TIM1->TCR = 1;
			}		
			else
			{				
				LPC_ADC->CR |= 1 << 24;
			}
			break;
		}
		
		case TS_Y1_MEASURE:
		{
			X_temp += 1023UL - Data;
			if(++Samples >= TS_SAMPLES)
			{
				Samples = 0;
				State = TS_Y2_SETUP_DLY;
				// X2 = 0, X1 = 1;
				TS_X2_FCLR = TS_X2_MASK;
				TS_X1_FSET = TS_X1_MASK;
				// Init setup delay
				LPC_TIM1->MR0 = TS_SETUP_DLY;
				LPC_TIM1->TCR = 1;
			}
			else
			{				
				LPC_ADC->CR |= 1 << 24;
			}
			break;
		}
			
		case TS_Y2_MEASURE:
		{
			X_temp += Data;
			if(++Samples >= TS_SAMPLES)
			{
				State = TS_INTR_SETUP_DLY;

				// Y1 = 1, Y2 = 1;
				TS_Y1_FSET  = TS_Y1_MASK;
				TS_Y2_FSET  = TS_Y2_MASK;
				TS_Y1_FDIR |= TS_Y1_MASK;
				TS_Y2_FDIR |= TS_Y2_MASK;
				TS_Y1_IOCON   = 0x1A0;

				// X1 - ADC Ch1, X2 input with pull down
				TS_X1_FDIR &= ~TS_X1_MASK;
				TS_X2_FDIR &= ~TS_X2_MASK;
				TS_X1_IOCON  = 0x101;          // assign to ADC0 Ch1
				TS_X2_IOCON  = 0x28;            // enable pull-down
				
				LPC_ADC->CR &= ~0xFF;
				LPC_ADC->CR |= 1UL<<1;    // select Ch1

				// Init setup delay
				LPC_TIM1->MR0 = TS_SAMPLE_DLY;
				LPC_TIM1->TCR = 1;
				Touch_temp = TRUE;
			}
			else
			{				
				LPC_ADC->CR |= 1 << 24;
			}
			break;
		}
		default:
		{
			break;
		}
	}
	NVIC->ICPR[0] |= 1 << 22;		
}

//*****************************************************************************
//! Detects if a touch accurs and offers the touch coordinates
//! \param pData Pointer to the \ref TouchResult structure where the function stores the touch coordinates
//! \return \b FALSE if no touch occured, \b TRUE if touch occured
//*****************************************************************************
BOOLEAN TouchGet(TouchResult* pData)
{
	BOOLEAN  TouchResReadyHold = Touch;
	uint32_t X_coordinate, Y_coordinate;
	if(TouchResReadyHold)
	{
		X_coordinate = X;
		Y_coordinate = Y;
		// Calculate X coordinate in pixels
		if (X_coordinate <= LEFT_UP_X)
		{
		  X_coordinate = 0;
		}
		else if (X_coordinate >= RIGHT_UP_X)
		{
		  X_coordinate = IMAGE_WIDTH;
		}
		else
		{
		  X_coordinate = ((X_coordinate - LEFT_UP_X) * IMAGE_WIDTH)/(RIGHT_UP_X-LEFT_UP_X);
		}
		// Calculate Y coordinate in pixels
		if (Y_coordinate <= LEFT_UP_Y)
		{
		  Y_coordinate = 0;
		}
		else if (Y_coordinate >= RIGHT_UP_Y)
		{
		  Y_coordinate = IMAGE_HEIGHT;
		}
		else
		{
		  Y_coordinate = ((Y_coordinate - LEFT_UP_Y) * IMAGE_HEIGHT)/(RIGHT_UP_Y-LEFT_UP_Y);
		}
		pData->X = X_coordinate;
		pData->Y = C_GLCD_V_SIZE - Y_coordinate;
		
	}
	return(TouchResReadyHold);
}


//*****************************************************************************
//! Set the touch event callback
//! 
//! This callback signals when a touchscreen event is generated
//! \param callback The user touch callback
//! \return The result of the operation. See \ref STATUS
//*****************************************************************************
STATUS DRV_TOUCHSCREEN_SetTouchCallback(TOUCHSCREEN_CallBack callback)
{
	currentTouchScreenCallback = callback;
	return OK;
}

//*****************************************************************************
//! TouchScreen driver main process function. This function is MANDATORY to be called in the program's main loop
//!
//! \return None.
//*****************************************************************************
void DRV_TOUCHSCREEN_Process()
{
	if (TouchGet(&touchResultData) == TRUE)
	{
		if (currentTouchScreenCallback != NULL)
		{
			(currentTouchScreenCallback)(&touchResultData);
		}
	}
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
