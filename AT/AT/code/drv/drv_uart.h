//*****************************************************************************
//! \file	drv_uart.h
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	UART Driver
//! 
//! Contains the driver for the uart modules of the microcontroller
//*****************************************************************************

#ifndef __DRV_UART_H
#define __DRV_UART_H

#include <stdint.h>
#include <utils/ringbuf.h>
#include <utils/timer_software.h>
#include "drv_general.h"
//*****************************************************************************
//! \addtogroup DRV_UART
//! @{
//*****************************************************************************


//*****************************************************************************
//! \enum UART
//! \typedef UART
//! Identify the UART module
//*****************************************************************************
typedef enum
{
	UART_0 = 0x00, /**< Module Uart 0  */
	UART_1 = 0x01, /**< Module Uart 1  */
	UART_2 = 0x02, /**< Module Uart 2  */
	UART_3 = 0x03, /**< Module Uart 3  */
	UART_4 = 0x04, /**< Module Uart 4  */
}UART;

//*****************************************************************************
//! \typedef UART_TX_CALLBACK
//! Uart transmission callback function type. The user callback function is mandatory to have this signature:
//!
//! void (*UART_TX_CALLBACK)(UART uart, STATUS status);
//*****************************************************************************
typedef void (*UART_TX_CALLBACK)(UART uart, STATUS status);


//*****************************************************************************
//! \typedef UART_RX_CALLBACK
//! Uart transmission callback function type. The user callback function is mandatory to have this signature:
//!
//! void (*UART_RX_CALLBACK)(UART uart, uint32_t size);
//*****************************************************************************
typedef void (*UART_RX_CALLBACK)(UART uart, uint32_t size);

//*****************************************************************************
//! \typedef UART_RX_TRIGGER
//! Uart transmission callback function type. The user trigger function is mandatory to have this signature:
//!
//! BOOLEAN (*UART_RX_TRIGGER)(UART uart, uint8_t byte);
//*****************************************************************************
typedef BOOLEAN (*UART_RX_TRIGGER)(UART uart, uint8_t byte);

//*****************************************************************************
//! \enum UART_PARITY
//! \typedef UART_PARITY
//! Uart Parity Modes
//*****************************************************************************
typedef enum
{
	UART_PARITY_ODD_PARITY		= 0x00, /**< Parity Enabled. Odd Parity  */
	UART_PARITY_EVEN_PARITY		= 0x01, /**< Parity Enabled. Even Parity  */
	UART_PARITY_FORCE_0			= 0x02, /**< Parity Enabled. Parity force at 0 */
	UART_PARITY_FORCE_1			= 0x03, /**< Parity Enabled. Parity force at 1 */
	UART_PARITY_NO_PARITY		= 0x04, /**< Parity Disabled */
}UART_PARITY;

//*****************************************************************************
//! \enum UART_CHARACTER_LENGTH
//! \typedef UART_CHARACTER_LENGTH
//! Uart Character Length
//*****************************************************************************
typedef enum
{
	UART_CHARACTER_LENGTH_5 = 0x00, /**< 5 bit per character */
	UART_CHARACTER_LENGTH_6 = 0x01,	/**< 6 bit per character */
	UART_CHARACTER_LENGTH_7 = 0x02, /**< 7 bit per character */
	UART_CHARACTER_LENGTH_8 = 0x03,	/**< 8 bit per character */
}UART_CHARACTER_LENGTH;

//*****************************************************************************
//! \enum UART_ERROR_TYPE
//! \typedef UART_ERROR_TYPE
//! Uart Error Type
//*****************************************************************************
typedef enum
{
	UART_ERROR_NO_ERROR,				/**< No Error */
	UART_ERROR_READ_TIMEOUT,			/**< Read Timeout */
	UART_ERROR_WRITE_TIMEOUT,			/**< Write Timeout */
	UART_ERROR_CONFIG_ERROR,			/**< Configuration Error */
	UART_ERROR_BUFFER_LIMITS_EXCEEDED,  /**< Buffers exceeded */
	UART_ERROR_BUFFERING_DISABLED,		/**< Uart buffering is disabled and no buffer related operations may be used */
}UART_ERROR_TYPE;

//*****************************************************************************
//! \struct UART_MAP
//! Structure that holds various uart information. Used internally for mapping the settings to the uart module
//! The structure is initialized internally for each uart. No user intervention is required
//! \private
//*****************************************************************************
typedef struct
{
	UART uartPort;							/**< Identifies the UART module */
	BOOLEAN isOpen;							/**< Specifies if the coresponding UART port si configured, opened and ready to use. Used internally by the driver */
	tRingBufObject *uart_rx_ring_buffer;    /**< Holds a pointer to the uart receive ring buffer */
	tRingBufObject *uart_tx_ring_buffer;	/**< Holds a pointer to the uart transmit ring buffer */
	uint8_t* uart_rx_buffer;				/**< Holds a pointer to the memory zone actually used by the uart receive ring buffer */
	uint8_t* uart_tx_buffer;				/**< Holds a pointer to the memory zone actually used by the uart transmit ring buffer */
	uint16_t uart_rx_buffer_size;			/**< Holds the size of the memory zone used by the uart receive ring buffer */
	uint16_t uart_tx_buffer_size;			/**< Holds the size of the memory zone used by the uart transmit ring buffer */
	UART_ERROR_TYPE uart_errno;				/**< Holds a variable conaining the last error reported by the uart driver */
	timer_software_handler_t uart_timer;	/**< Holds the software timer handler needed by the uart driver for the specified module to implement the timeout system  */
	BOOLEAN uart_buffering_enabled;			/**< Specifies if the buffering mode of the uart is enabled  */
	uint32_t read_write_timeout_ms;			/**< Holds the actual value of the uart timeout (in milliseconds  */
	BOOLEAN trigger;						/**< Specified if the receive callback action has been triggered. Used internally by the driver in case receive triggering is activated*/
	UART_TX_CALLBACK uart_tx_callback;		/**< Holds the pointer to the transmission callback function */
	UART_RX_CALLBACK uart_rx_callback;		/**< Holds the pointer to the receive callback function */
	UART_RX_TRIGGER uart_rx_trigger;		/**< Holds the pointer to the receive trigger function */		
}UART_MAP;

void DRV_UART_SetTimeout(UART uart, uint32_t read_write_timeout_ms);
uint32_t DRV_UART_GetTimeout(UART uart);

void DRV_UART_Configure(UART uart, UART_CHARACTER_LENGTH charLength, uint32_t baud, UART_PARITY parity, uint8_t stopBits, BOOLEAN buffered, uint32_t read_write_timeout_ms);
STATUS DRV_UART_SendCharBlocking(UART uart, uint8_t data);
STATUS DRV_UART_GetCharBlocking(UART uart, uint8_t *dest);
STATUS DRV_UART_SendByte(UART uart, uint8_t byte);
STATUS DRV_UART_ReadByte(UART uart, uint8_t *dst);

uint32_t DRV_UART_BytesAvailable(UART uart);
void DRV_UART_SendBufferBlocking(UART uart, uint8_t *buffer, uint16_t size);
uint32_t DRV_UART_GetTimeout(UART uart);
void DRV_UART_SetTimeout(UART uart, uint32_t read_write_timeout_ms);
UART_ERROR_TYPE DRV_UART_GetErrno(UART uart);
uint32_t DRV_UART_Read(UART uart, uint8_t *dest, uint32_t count);
uint32_t DRV_UART_Write(UART uart, uint8_t *src, uint32_t count);
void DRV_UART_Process(void);
STATUS DRV_UART_SetRxCallback(UART uart_port, UART_RX_CALLBACK callback);
STATUS DRV_UART_SetTxCallback(UART uart_port, UART_TX_CALLBACK callback);
STATUS DRV_UART_SetRxTriggerFunction(UART uart_port, UART_RX_TRIGGER function);
STATUS DRV_UART_FlushRX(UART uart_port);
STATUS DRV_UART_FlushTX(UART uart_port);

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************


#endif
