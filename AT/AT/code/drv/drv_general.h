//*****************************************************************************
//! \file	drv_general.h
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	General structures, types, functions and defines
//! 
//! Contains various types, functions and defines used by the rest of this driver library
//*****************************************************************************


#ifndef __DRV_GENERAL_H
#define __DRV_GENERAL_H

//*****************************************************************************
//! \addtogroup DRV_GENERAL
//! @{
//*****************************************************************************

#define MHZ           *1000000l	/**< Define for MHz  */
#define KHZ           *1000l	/**< Define for KHz  */
#define HZ            *1l		/**< Define for Hz  */

#define MAX(x,y) 	( (x > y) ? x : y )			/**< Define macro for the maximum value between two numbers  */
#define MIN(x,y) 	( (x < y) ? x : y )			/**< Define macro for the minimum value between two numbers*/
#define ABS(x)	 	( (x > 0) ? x : (-x) )		/**< Define macro for the absolute value of a number  */

#ifndef NULL
#define NULL 0
#endif

#include <stdint.h>

#define UART0_TX_BUFFER_SIZE 512 /**< Defines the transmit buffer size for UART 0  */
#define UART1_TX_BUFFER_SIZE 512 /**< Defines the transmit buffer size for UART 1  */
#define UART2_TX_BUFFER_SIZE 512 /**< Defines the transmit buffer size for UART 2  */
#define UART3_TX_BUFFER_SIZE 512 /**< Defines the transmit buffer size for UART 3  */
#define UART4_TX_BUFFER_SIZE 512 /**< Defines the transmit buffer size for UART 4  */

#define UART0_RX_BUFFER_SIZE 512 /**< Defines the receive  buffer size for UART 0  */
#define UART1_RX_BUFFER_SIZE 512 /**< Defines the receive  buffer size for UART 1  */
#define UART2_RX_BUFFER_SIZE 512 /**< Defines the receive  buffer size for UART 2  */
#define UART3_RX_BUFFER_SIZE 512 /**< Defines the receive  buffer size for UART 3  */
#define UART4_RX_BUFFER_SIZE 512 /**< Defines the receive  buffer size for UART 4  */

#define UART_RX_FIFO_TRIGGER 0


//*****************************************************************************
//! \enum BOOLEAN
//! \typedef BOOLEAN
//! Defines the a boolean type
//! This structure uses DLD as the fractional divider
//*****************************************************************************
typedef enum
{
	FALSE = 0x00, /**< False boolean value  */
	TRUE = 0x01,  /**< True boolean value  */
}BOOLEAN;

//*****************************************************************************
//! \struct UART_BAUDRATE_VALUE_DLD
//! The structure used for encapsulating the values used for the uart divisor in order to obtain a specific baudrate
//! This structure uses DLD as the fractional divider
//
//*****************************************************************************
typedef struct
{
	uint8_t DLM;  /*!< The DLM value - most significant byte of the integer divider*/
	uint8_t DLL;  /*!< The DLL value - least significant byte of the integer divider*/
	uint8_t DLD;  /*!< The DLD value - fractional divider*/
}UART_BAUDRATE_VALUE_DLD;


//*****************************************************************************
//! \struct UART_BAUDRATE_VALUE_DIV_MUL
//! The structure used for encapsulating the values used for the uart divisor in order to obtain a specific baudrate
//! This structure uses mul and div for the fractional divider (fractional divider = (mval / dval)
//
//*****************************************************************************
typedef struct
{
	uint8_t DLM;  /*!< The DLM value - most significant byte of the integer divider*/
	uint8_t DLL;  /*!< The DLL value - least significant byte of the integer divider*/
	uint8_t mval; /*!< The numerator of the fractional divider*/
	uint8_t dval; /*!< The denominator of the fractional divider*/
}UART_BAUDRATE_VALUE_DIV_MUL;

#define UART_LOAD_DLM(div)  (((div) >> 8) & 0xFF)    /**< Macro for loading most significant halfs of divisors */
#define UART_LOAD_DLL(div)    ((div) & 0xFF)    /**< Macro for loading least significant halfs of divisors */

//*****************************************************************************
//! \enum STATUS
//! \typedef STATUS
//! Defines a status return type
//! This enum defines the possible return status types of the functions defined by this library
//*****************************************************************************
typedef enum 
{
	OK 					= 0,  /**< Operation finished ok  */
	NOT_OK 				= 1,  /**< Operation finished not ok  */
	EMPTY_QUEUE 		= 2,  /**< The queue/buffer is empty  */
	BUSY 				= 3,  /**< The called module is busy  */
	IDLE 				= 4,  /**< The called module is empty  */
	INCORRECT_PARAMS 	= 5,  /**< The parameters are incorrect */
	TIMEOUT 			= 6,  /**< Timeout occured in the called module  */
	RESERVED_6	 		= 7, 
	RESERVED_8 			= 8,
	NOT_INIT 			= 9,  /**< The module was not initialized  */
	FULL_QUEUE 			= 10, /**< The queue/buffer is full  */
	END_OF_QUEUE 		= 11, /**< End of queue/buffer was reached  */
	CRC_ERROR 			= 12, /**< CRC check failed  */
	OUT_OF_SYNC 		= 13, /**< Operation was out of sync  */
	INCORRECT_PASSWORD 	= 14, /**< Incorrect password  */
	NO_TIMERS_AVAILABLE = 15, /**< No more software timers available  */
	NOT_READY			= 16, /**< The called module is not ready  */
}STATUS;

UART_BAUDRATE_VALUE_DLD DRV_GENERAL_CalculateBaudrateDLD(uint32_t PeripheralClock, uint32_t baud, uint8_t sampling);
UART_BAUDRATE_VALUE_DIV_MUL	 DRV_GENERAL_CalculateBaudrateDivMul(uint32_t PeripheralClock, uint32_t baudrate, uint8_t sampling);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#endif
