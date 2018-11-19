//*****************************************************************************
//! \file	drv_uart.c
//! \author	Valentin STANGACIU, DSPLabs
//! 
//! \brief	UART Driver
//! 
//! Contains the driver for the uart modules of the microcontroller
//*****************************************************************************

//*****************************************************************************
//! \headerfile drv_uart.h "drv_uart.h"
//*****************************************************************************

//*****************************************************************************
//! \addtogroup DRV_UART
//! @{
//! \brief	UART Driver
//! 
//! Contains the driver for the uart modules of the microcontroller
//*****************************************************************************

#include "drv_uart.h"
#include <stdint.h>
#include <assert.h>
#include <utils/ringbuf.h>
#include <utils/timer_software.h>
#include "LPC177x_8x.h"
#include "system_LPC177x_8x.h"
#include "drv_general.h"

static uint8_t uart0_rx_buffer[UART0_RX_BUFFER_SIZE];	/*!< Receive buffer for UART 0*/
static uint8_t uart1_rx_buffer[UART1_RX_BUFFER_SIZE];	/*!< Receive buffer for UART 1*/
static uint8_t uart2_rx_buffer[UART2_RX_BUFFER_SIZE];	/*!< Receive buffer for UART 2*/
static uint8_t uart3_rx_buffer[UART3_RX_BUFFER_SIZE];	/*!< Receive buffer for UART 3*/
static uint8_t uart4_rx_buffer[UART4_RX_BUFFER_SIZE];	/*!< Receive buffer for UART 4*/

static uint8_t uart0_tx_buffer[UART0_TX_BUFFER_SIZE];	/*!< Transmit buffer for UART 0*/
static uint8_t uart1_tx_buffer[UART1_TX_BUFFER_SIZE];	/*!< Transmit buffer for UART 1*/
static uint8_t uart2_tx_buffer[UART2_TX_BUFFER_SIZE];	/*!< Transmit buffer for UART 2*/
static uint8_t uart3_tx_buffer[UART3_TX_BUFFER_SIZE];	/*!< Transmit buffer for UART 3*/
static uint8_t uart4_tx_buffer[UART4_TX_BUFFER_SIZE];	/*!< Transmit buffer for UART 4*/

static tRingBufObject uart0_rx_ring_buffer;		/*!< Receive ring buffer handler for UART 0*/
static tRingBufObject uart1_rx_ring_buffer;		/*!< Receive ring buffer handler for UART 1*/
static tRingBufObject uart2_rx_ring_buffer;		/*!< Receive ring buffer handler for UART 2*/
static tRingBufObject uart3_rx_ring_buffer;		/*!< Receive ring buffer handler for UART 3*/
static tRingBufObject uart4_rx_ring_buffer;		/*!< Receive ring buffer handler for UART 4*/

static tRingBufObject uart0_tx_ring_buffer;		/*!< Transmit ring buffer handler for UART 0*/
static tRingBufObject uart1_tx_ring_buffer;		/*!< Transmit ring buffer handler for UART 1*/
static tRingBufObject uart2_tx_ring_buffer;		/*!< Transmit ring buffer handler for UART 2*/
static tRingBufObject uart3_tx_ring_buffer;		/*!< Transmit ring buffer handler for UART 3*/
static tRingBufObject uart4_tx_ring_buffer;		/*!< Transmit ring buffer handler for UART 4*/

//*****************************************************************************
/*! \var UART_MAP uartMaps[]
	\brief UART Mapping Table
*/
//*****************************************************************************
UART_MAP uartMaps[] = 
						{
							{UART_0, FALSE, &uart0_rx_ring_buffer, &uart0_tx_ring_buffer, uart0_rx_buffer, uart0_tx_buffer, UART0_RX_BUFFER_SIZE, UART0_TX_BUFFER_SIZE, UART_ERROR_NO_ERROR, 0, FALSE, 0, FALSE, NULL, NULL, NULL},
							{UART_1, FALSE, &uart1_rx_ring_buffer, &uart1_tx_ring_buffer, uart1_rx_buffer, uart1_tx_buffer, UART1_RX_BUFFER_SIZE, UART1_TX_BUFFER_SIZE, UART_ERROR_NO_ERROR, 0, FALSE, 0, FALSE, NULL, NULL, NULL},
							{UART_2, FALSE, &uart2_rx_ring_buffer, &uart2_tx_ring_buffer, uart2_rx_buffer, uart2_tx_buffer, UART2_RX_BUFFER_SIZE, UART2_TX_BUFFER_SIZE, UART_ERROR_NO_ERROR, 0, FALSE, 0, FALSE, NULL, NULL, NULL},
							{UART_3, FALSE, &uart3_rx_ring_buffer, &uart3_tx_ring_buffer, uart3_rx_buffer, uart3_tx_buffer, UART3_RX_BUFFER_SIZE, UART3_TX_BUFFER_SIZE, UART_ERROR_NO_ERROR, 0, FALSE, 0, FALSE, NULL, NULL, NULL},
							{UART_4, FALSE, &uart4_rx_ring_buffer, &uart4_tx_ring_buffer, uart4_rx_buffer, uart4_tx_buffer, UART4_RX_BUFFER_SIZE, UART4_TX_BUFFER_SIZE, UART_ERROR_NO_ERROR, 0, FALSE, 0, FALSE, NULL, NULL, NULL},
						};

//*****************************************************************************
//! General Interrupt handler routine 
//!
//! This function is the interrupt handling routing for all uart modules. It is called by each module's private interrupt handling routine
//! \param uart Identified the uart module
//! \return None.
//! \private						
//*****************************************************************************
void DRV_UART_GeneralIRQHandler(UART uart)
{
	uint8_t data = 0x00;
	uint32_t iir = 0;
	switch (uart)
	{
		case UART_0:
		{
			iir = LPC_UART0->IIR;
			if (iir & (0x2 << 1))
			{
				// rx interrupt
				data = LPC_UART0->RBR;
				if (!RingBufFull(&uart0_rx_ring_buffer))
				{
					RingBufWriteOne(&uart0_rx_ring_buffer, data);
				}
				if (uartMaps[UART_0].uart_rx_callback != NULL)
				{
					if (uartMaps[UART_0].uart_rx_trigger != NULL)
					{
						uartMaps[UART_0].trigger = (uartMaps[UART_0].uart_rx_trigger)(UART_0, data);
					}
					else
					{
						uartMaps[UART_0].trigger = TRUE;
					}
				}
				
			}
			if (iir & (0x1 << 1))
			{
				// tx interrupt
				if (RingBufEmpty(&uart0_tx_ring_buffer))
				{
					LPC_UART0->IER &= ~2;
				}
				else
				{
					data = RingBufReadOne(&uart0_tx_ring_buffer);
					LPC_UART0->THR = data;
					if (RingBufEmpty(&uart0_tx_ring_buffer))
					{
						LPC_UART0->IER &= ~2;
					}	
				}				
			}
			break;
		}
		
		case UART_1:
		{
			iir = LPC_UART1->IIR;
			if (iir & (0x2 << 1))
			{
				// rx interrupt
				data = LPC_UART1->RBR;
				if (!RingBufFull(&uart1_rx_ring_buffer))
				{
					RingBufWriteOne(&uart1_rx_ring_buffer, data);
				}
				if (uartMaps[UART_1].uart_rx_callback != NULL)
				{
					if (uartMaps[UART_1].uart_rx_trigger != NULL)
					{
						uartMaps[UART_1].trigger = (uartMaps[UART_1].uart_rx_trigger)(UART_0, data);
					}
					else
					{
						uartMaps[UART_1].trigger = TRUE;
					}
				}
				
			}
			if (iir & (0x1 << 1))
			{
				// tx interrupt
				if (RingBufEmpty(&uart1_tx_ring_buffer))
				{
					LPC_UART1->IER &= ~2;
				}
				else
				{
					data = RingBufReadOne(&uart1_tx_ring_buffer);
					LPC_UART1->THR = data;
					if (RingBufEmpty(&uart1_tx_ring_buffer))
					{
						LPC_UART1->IER &= ~2;
					}	
				}				
			}
			break;
		}
		
		case UART_2:
		{
			iir = LPC_UART2->IIR;
			if (iir & (0x2 << 1))
			{
				// rx interrupt
				data = LPC_UART2->RBR;
				if (!RingBufFull(&uart2_rx_ring_buffer))
				{
					RingBufWriteOne(&uart2_rx_ring_buffer, data);
				}
				if (uartMaps[UART_2].uart_rx_callback != NULL)
				{
					if (uartMaps[UART_2].uart_rx_trigger != NULL)
					{
						uartMaps[UART_2].trigger = (uartMaps[UART_2].uart_rx_trigger)(UART_0, data);
					}
					else
					{
						uartMaps[UART_2].trigger = TRUE;
					}
				}
				
			}
			if (iir & (0x1 << 1))
			{
				// tx interrupt
				if (RingBufEmpty(&uart2_tx_ring_buffer))
				{
					LPC_UART2->IER &= ~2;
				}
				else
				{
					data = RingBufReadOne(&uart2_tx_ring_buffer);
					LPC_UART2->THR = data;
					if (RingBufEmpty(&uart2_tx_ring_buffer))
					{
						LPC_UART2->IER &= ~2;
					}	
				}				
			}
			break;
		}
		
		case UART_3:
		{
			iir = LPC_UART3->IIR;
			if (iir & (0x2 << 1))
			{
				// rx interrupt
				data = LPC_UART3->RBR;
				if (!RingBufFull(&uart3_rx_ring_buffer))
				{
					RingBufWriteOne(&uart3_rx_ring_buffer, data);
				}
				if (uartMaps[UART_3].uart_rx_callback != NULL)
				{
					if (uartMaps[UART_3].uart_rx_trigger != NULL)
					{
						uartMaps[UART_3].trigger = (uartMaps[UART_3].uart_rx_trigger)(UART_3, data);
					}
					else
					{
						uartMaps[UART_3].trigger = TRUE;
					}
				}
				
			}
			if (iir & (0x1 << 1))
			{
				// tx interrupt
				if (RingBufEmpty(&uart3_tx_ring_buffer))
				{
					LPC_UART3->IER &= ~2;
				}
				else
				{
					data = RingBufReadOne(&uart3_tx_ring_buffer);
					LPC_UART3->THR = data;
					if (RingBufEmpty(&uart3_tx_ring_buffer))
					{
						LPC_UART3->IER &= ~2;
					}	
				}				
			}
			break;
		}
		
		case UART_4:
		{
			iir = LPC_UART4->IIR;
			if (iir & (0x2 << 1))
			{
				// rx interrupt
				data = LPC_UART4->RBR;
				if (!RingBufFull(&uart4_rx_ring_buffer))
				{
					RingBufWriteOne(&uart4_rx_ring_buffer, data);
				}
				if (uartMaps[UART_4].uart_rx_callback != NULL)
				{
					if (uartMaps[UART_4].uart_rx_trigger != NULL)
					{
						uartMaps[UART_4].trigger = (uartMaps[UART_4].uart_rx_trigger)(UART_0, data);
					}
					else
					{
						uartMaps[UART_4].trigger = TRUE;
					}
				}
				
			}
			if (iir & (0x1 << 1))
			{
				// tx interrupt
				if (RingBufEmpty(&uart4_tx_ring_buffer))
				{
					LPC_UART4->IER &= ~2;
				}
				else
				{
					data = RingBufReadOne(&uart4_tx_ring_buffer);
					LPC_UART4->THR = data;
					if (RingBufEmpty(&uart4_tx_ring_buffer))
					{
						LPC_UART4->IER &= ~2;
					}	
				}				
			}
			break;
		}
	}
}

//*****************************************************************************
//! Interrupt handling routine for UART 0
//!
//! This function is the interrupt handling routing for UART0. It calls the general interrupt handling routine using UART_0 as the uart parameter
//! \return None.
//! \private						
//*****************************************************************************
void UART0_IRQHandler()
{
	DRV_UART_GeneralIRQHandler(UART_0);
}

//*****************************************************************************
//! Interrupt handling routine for UART 1
//!
//! This function is the interrupt handling routing for UART1. It calls the general interrupt handling routine using UART_1 as the uart parameter
//! \return None.
//! \private						
//*****************************************************************************
void UART1_IRQHandler()
{
	DRV_UART_GeneralIRQHandler(UART_1);
}

//*****************************************************************************
//! Interrupt handling routine for UART 2
//!
//! This function is the interrupt handling routing for UART2. It calls the general interrupt handling routine using UART_2 as the uart parameter
//! \return None.
//! \private						
//*****************************************************************************
void UART2_IRQHandler()
{
	DRV_UART_GeneralIRQHandler(UART_2);
}

//*****************************************************************************
//! Interrupt handling routine for UART 3
//!
//! This function is the interrupt handling routing for UART3. It calls the general interrupt handling routine using UART_3 as the uart parameter
//! \return None.
//! \private						
//*****************************************************************************
void UART3_IRQHandler()
{
	DRV_UART_GeneralIRQHandler(UART_3);
}

//*****************************************************************************
//! Interrupt handling routine for UART 4
//!
//! This function is the interrupt handling routing for UART4. It calls the general interrupt handling routine using UART_4 as the uart parameter
//! \return None.
//! \private						
//*****************************************************************************
void UART4_IRQHandler()
{
	DRV_UART_GeneralIRQHandler(UART_4);
}

//*****************************************************************************
//! Set the read/write timeout
//!
//! This functions sets the timeout for the read and write operations of the selected uart module. If the timeout value is 0 then the timeout system is disabled and all functions execute in a blocking manner
//! \param uart The UART module.
//! \param read_write_timeout_ms The desired timeout in milliseconds
//! \return None.
//*****************************************************************************
void DRV_UART_SetTimeout(UART uart, uint32_t read_write_timeout_ms)
{
	assert(uart <= UART_4);
	uartMaps[uart].read_write_timeout_ms = read_write_timeout_ms;
	TIMER_SOFTWARE_configure_timer(uartMaps[uart].uart_timer, MODE_0, uartMaps[uart].read_write_timeout_ms, 1);
	TIMER_SOFTWARE_stop_timer(uartMaps[uart].uart_timer);
	TIMER_SOFTWARE_reset_timer(uartMaps[uart].uart_timer);
	TIMER_SOFTWARE_clear_interrupt(uartMaps[uart].uart_timer);
}

//*****************************************************************************
//! Get the read/write timeout
//!
//! This functions returns the value of the current read write timeout.
//! \param uart The UART module.
//! \return The current value of the read write timeout in milliseconds
//*****************************************************************************
uint32_t DRV_UART_GetTimeout(UART uart)
{
	assert(uart <= UART_4);	
	return uartMaps[uart].read_write_timeout_ms;
}

//*****************************************************************************
//! Configure the uart module.
//!
//! This function is used to configure the uart module. All parameters are asserted inside the functions for correct values.
//!
//! After calling this function the status of the configuration may be checked via the errno variable for each uart module using \ref DRV_UART_GetErrno
//! \param uart The UART module to be configured.
//! \param charLength The length of the character. The number of bits per character. See \ref UART_CHARACTER_LENGTH for possible values.
//! \param baud The desired baudrate for the selected uart module. Can be any value. The divisors are calculated even for non standard baudrates.
//! \param parity Specifies if the parity should be enabled or disabled. If enabled it specifies the parity mode. See \ref UART_PARITY for possible values.
//! \param stopBits Specifies the number of stop bits. May be 1 or 2. Depending on the uart configuration there may be a stopBits value of 2 actually represeting 1.5 bits for stop. 
//! \param buffered Specifies if buffering is enabled. If value is \b FALSE then the timeout system is disabled and all functions operating in a blocking manner and only character based functions are enabled.
//! \param read_write_timeout_ms The desired timeout in milliseconds. If the value is 0 then the timeout system is disabled and all functions operating in a blocking manner.
//! \return None.
//*****************************************************************************
void DRV_UART_Configure(UART uart, UART_CHARACTER_LENGTH charLength, uint32_t baud, UART_PARITY parity, uint8_t stopBits, BOOLEAN buffered, uint32_t read_write_timeout_ms)
{
	UART_BAUDRATE_VALUE_DIV_MUL baudRate;
	int16_t temp;
	assert(uart <= UART_4);
	assert(parity <= UART_PARITY_NO_PARITY);
	assert(stopBits > 0);
	assert(stopBits <= 2);
	assert(charLength <= UART_CHARACTER_LENGTH_8);
	baudRate = DRV_GENERAL_CalculateBaudrateDivMul(PeripheralClock, baud, 16);	
	uartMaps[uart].uart_tx_callback = NULL;
	uartMaps[uart].uart_rx_callback = NULL;
	uartMaps[uart].uart_rx_trigger = NULL;
	uartMaps[uart].trigger = FALSE;
	switch (uart)
	{
		case UART_0:
		{
			uartMaps[UART_0].uart_buffering_enabled = buffered;
			LPC_IOCON->P0_3 = 1;
			LPC_IOCON->P0_2 = 1;
			LPC_SC->PCONP |= 1 << 3;
			LPC_UART0->LCR = 0;
			LPC_UART0->LCR = 1 << 7; // dlab - enable access to divisor
			LPC_UART0->DLM = baudRate.DLM;
			LPC_UART0->DLL = baudRate.DLL;
			LPC_UART0->FDR = baudRate.dval & 0x0F;
			LPC_UART0->FDR |= (baudRate.mval & 0x0F) << 4;
			LPC_UART0->LCR = charLength | (stopBits < 2);
			LPC_UART0->FCR = 1 | (UART_RX_FIFO_TRIGGER << 6);
			if (parity != UART_PARITY_NO_PARITY)
			{
				LPC_UART0->LCR |= 1 << 3; // enable parity
				LPC_UART0->LCR |= (parity & 3) << 4; // select parity type
			}
			if (uartMaps[UART_0].uart_buffering_enabled == TRUE)
			{
				temp = TIMER_SOFTWARE_request_timer();
				assert(temp >= 0);
				uartMaps[UART_0].uart_timer = temp;
				DRV_UART_SetTimeout(uart, read_write_timeout_ms);
				RingBufInit(uartMaps[UART_0].uart_rx_ring_buffer, uartMaps[UART_0].uart_rx_buffer, uartMaps[UART_0].uart_rx_buffer_size);  
				RingBufInit(uartMaps[UART_0].uart_tx_ring_buffer, uartMaps[UART_0].uart_tx_buffer, uartMaps[UART_0].uart_tx_buffer_size);
				LPC_UART0->IER &= ~2;
				LPC_UART0->IER |= 1;
				NVIC->ISER[0] |= 1 << 5;				
			}
			uartMaps[UART_0].uart_errno = UART_ERROR_NO_ERROR;
			break;
		}
		case UART_1:
		{
			break;
		}
		case UART_2:
		{
		
			uartMaps[UART_2].uart_buffering_enabled = buffered;
			LPC_IOCON->P4_23 = 2;
			LPC_IOCON->P4_22 = 2;
			LPC_SC->PCONP |= 1 << 24;
			LPC_UART2->LCR = 0;
			LPC_UART2->LCR = 1 << 7; // dlab - enable access to divisor
			LPC_UART2->DLM = baudRate.DLM;
			LPC_UART2->DLL = baudRate.DLL;
			LPC_UART2->FDR = baudRate.dval & 0x0F;
			LPC_UART2->FDR |= (baudRate.mval & 0x0F) << 4;
			LPC_UART2->LCR = charLength | (stopBits < 2);
			LPC_UART2->FCR = 1 | (UART_RX_FIFO_TRIGGER << 6);
			if (parity != UART_PARITY_NO_PARITY)
			{
				LPC_UART2->LCR |= 1 << 3; // enable parity
				LPC_UART2->LCR |= (parity & 3) << 4; // select parity type
			}
			if (uartMaps[UART_2].uart_buffering_enabled == TRUE)
			{
				temp = TIMER_SOFTWARE_request_timer();
				assert(temp >= 0);
				uartMaps[UART_2].uart_timer = temp;
				DRV_UART_SetTimeout(uart, read_write_timeout_ms);
				RingBufInit(uartMaps[UART_2].uart_rx_ring_buffer, uartMaps[UART_2].uart_rx_buffer, uartMaps[UART_2].uart_rx_buffer_size);  
				RingBufInit(uartMaps[UART_2].uart_tx_ring_buffer, uartMaps[UART_2].uart_tx_buffer, uartMaps[UART_2].uart_tx_buffer_size);
				LPC_UART2->IER &= ~2;
				LPC_UART2->IER |= 1;
				NVIC->ISER[0] |= 1 << 7;				
			}
			uartMaps[UART_2].uart_errno = UART_ERROR_NO_ERROR;
			break;
		}
		case UART_3:
		{
			uartMaps[UART_3].uart_buffering_enabled = buffered;
			LPC_IOCON->P0_25 = 3 | (1 << 7);
			LPC_IOCON->P0_26 = 3 | (1 << 7);
			LPC_SC->PCONP |= 1 << 25;
			LPC_UART3->LCR = 0;
			LPC_UART3->LCR = 1 << 7; // dlab - enable access to divisor
			LPC_UART3->DLM = baudRate.DLM;
			LPC_UART3->DLL = baudRate.DLL;
			LPC_UART3->FDR = baudRate.dval & 0x0F;
			LPC_UART3->FDR |= (baudRate.mval & 0x0F) << 4;
			LPC_UART3->LCR = charLength | (stopBits < 2);
			LPC_UART3->FCR = 1 | (UART_RX_FIFO_TRIGGER << 6);
			if (parity != UART_PARITY_NO_PARITY)
			{
				LPC_UART3->LCR |= 1 << 3; // enable parity
				LPC_UART3->LCR |= (parity & 3) << 4; // select parity type
			}
			if (uartMaps[UART_3].uart_buffering_enabled == TRUE)
			{
				temp = TIMER_SOFTWARE_request_timer();
				assert(temp >= 0);
				uartMaps[UART_3].uart_timer = temp;
				DRV_UART_SetTimeout(uart, read_write_timeout_ms);
				RingBufInit(uartMaps[UART_3].uart_rx_ring_buffer, uartMaps[UART_3].uart_rx_buffer, uartMaps[UART_3].uart_rx_buffer_size);  
				RingBufInit(uartMaps[UART_3].uart_tx_ring_buffer, uartMaps[UART_3].uart_tx_buffer, uartMaps[UART_3].uart_tx_buffer_size);
				LPC_UART3->IER &= ~2;
				LPC_UART3->IER |= 1;
				NVIC->ISER[0] |= 1 << 8;				
			}
			uartMaps[UART_3].uart_errno = UART_ERROR_NO_ERROR;
			break;
		}
		case UART_4:
		{
			break;
		}
	}
	uartMaps[uart].isOpen = TRUE;
}

//*****************************************************************************
//! Send a character through the selected uart module. This function blocks the execution of the program while transmitting the character.
//! 
//! After calling this function the status of the configuration may be checked via the errno variable for each uart module using \ref DRV_UART_GetErrno
//! \param uart The UART module.
//! \param data The data to be sent though the uart module
//! \return The result of the operation. See \ref STATUS.
//*****************************************************************************
STATUS DRV_UART_SendCharBlocking(UART uart, uint8_t data)
{
	assert(uart <= UART_4);
	if (!uartMaps[uart].isOpen)
	{
		return NOT_INIT;
	}
	switch (uart)
	{
		case UART_0:
		{
			LPC_UART0->THR = data;
			while (!(LPC_UART0->LSR & (1 << 5)));
			uartMaps[UART_0].uart_errno = UART_ERROR_NO_ERROR;
			break;
		}
		case UART_1:
		{
			LPC_UART1->THR = data;
			while (!(LPC_UART1->LSR & (1 << 5)));
			uartMaps[UART_1].uart_errno = UART_ERROR_NO_ERROR;
			break;
		}
		case UART_2:
		{
			LPC_UART2->THR = data;
			while (!(LPC_UART2->LSR & (1 << 5)));
			uartMaps[UART_2].uart_errno = UART_ERROR_NO_ERROR;
			break;
		}
		case UART_3:
		{
			LPC_UART3->THR = data;
			while (!(LPC_UART3->LSR & (1 << 5)));
			uartMaps[UART_3].uart_errno = UART_ERROR_NO_ERROR;
			break;
		}
		case UART_4:
		{
			LPC_UART4->THR = data;
			while (!(LPC_UART4->LSR & (1 << 5)));
			uartMaps[UART_4].uart_errno = UART_ERROR_NO_ERROR;
			break;
		}
	}
	return OK;
}

//*****************************************************************************
//! Send a buffer through the selected uart module. This function blocks the execution of the program while transmitting the buffer.
//! 
//! After calling this function the status of the configuration may be checked via the errno variable for each uart module using \ref DRV_UART_GetErrno.
//! \param uart The UART module.
//! \param buffer A pointer to the memory zone containing the buffer.
//! \param size The size of the memory zone buffer.
//! \return None.
//*****************************************************************************
void DRV_UART_SendBufferBlocking(UART uart, uint8_t* buffer, uint16_t size)
{
	uint16_t i;
	assert(uart <= UART_4);
	if (!uartMaps[uart].isOpen)
	{
		return;
	}	
	for (i = 0; i < size; i++)
	{
		DRV_UART_SendCharBlocking(uart, buffer[i]);
	}
}

//*****************************************************************************
//! Get a byte from the selected uart module. This function blocks the execution of the program until a byte is received by the uart module
//! 
//! \param uart The UART module.
//! \param dest Pointer where the read character will be stored
//! \return The received byte.
//*****************************************************************************
STATUS DRV_UART_GetCharBlocking(UART uart, uint8_t *dest)
{
	assert(uart <= UART_4);
	if (!uartMaps[uart].isOpen)
	{
		return NOT_OK;
	}
	switch (uart)
	{
		case UART_0:
		{
			while((LPC_UART0->LSR & 1) == 0);
			uartMaps[UART_0].uart_errno = UART_ERROR_NO_ERROR;
			*dest = LPC_UART0->RBR;
			break;
		}
		case UART_1:
		{
			while((LPC_UART1->LSR & 1) == 0);
			uartMaps[UART_1].uart_errno = UART_ERROR_NO_ERROR;
			*dest = LPC_UART1->RBR;
			break;
		}
		case UART_2:
		{
			while((LPC_UART2->LSR & 1) == 0);
			uartMaps[UART_2].uart_errno = UART_ERROR_NO_ERROR;
			*dest = LPC_UART2->RBR;
			break;
		}
		case UART_3:
		{
			while((LPC_UART3->LSR & 1) == 0);
			uartMaps[UART_3].uart_errno = UART_ERROR_NO_ERROR;
			*dest = LPC_UART3->RBR;
			break;
		}
		case UART_4:
		{
			while((LPC_UART4->LSR & 1) == 0);
			uartMaps[UART_4].uart_errno = UART_ERROR_NO_ERROR;
			*dest = LPC_UART4->RBR;
			break;
		}
	}
	return OK;
}

//*****************************************************************************
//! Send a byte through the selected uart using buffered mode. 
//! 
//! If the buffered mode is disabled then this function has no effect
//! 
//! After calling this function the status of the configuration may be checked via the errno variable for each uart module using \ref DRV_UART_GetErrno
//! \param uart The UART module.
//! \param byte The data to be sent though the uart module.
//! \return None.
//! \private
//*****************************************************************************
void DRV_UART_PrivateSendByte(UART uart, uint8_t byte)
{
	assert(uart <= UART_4);
	if (uartMaps[uart].uart_buffering_enabled == FALSE)
	{
		uartMaps[uart].uart_errno = UART_ERROR_BUFFERING_DISABLED;
		return;
	}
	switch (uart)
	{
		case UART_0:
		{
			if ((LPC_UART0->IER & 2) == 0)
			{
				LPC_UART0->IER |= 2;
				LPC_UART0->THR = byte;
			}
			else
			{
				RingBufWriteOne(uartMaps[UART_0].uart_tx_ring_buffer, byte);				
			}
			break;
		}
		case UART_1:
		{
			if ((LPC_UART1->IER & 2) == 0)
			{
				LPC_UART1->IER |= 2;
				LPC_UART1->THR = byte;
			}
			else
			{
				RingBufWriteOne(uartMaps[UART_1].uart_tx_ring_buffer, byte);				
			}
			break;
		}
		case UART_2:
		{
			if ((LPC_UART2->IER & 2) == 0)
			{
				LPC_UART2->IER |= 2;
				LPC_UART2->THR = byte;
			}
			else
			{
				RingBufWriteOne(uartMaps[UART_2].uart_tx_ring_buffer, byte);				
			}
			break;
		}
		case UART_3:
		{
			if ((LPC_UART3->IER & 2) == 0)
			{
				LPC_UART3->IER |= 2;
				LPC_UART3->THR = byte;
			}
			else
			{
				RingBufWriteOne(uartMaps[UART_3].uart_tx_ring_buffer, byte);				
			}
			break;
		}
		case UART_4:
		{
			if ((LPC_UART4->IER & 2) == 0)
			{
				LPC_UART4->IER |= 2;
				LPC_UART4->THR = byte;
			}
			else
			{
				RingBufWriteOne(uartMaps[UART_4].uart_tx_ring_buffer, byte);				
			}
			break;
		}
	}
}


//*****************************************************************************
//! Gets the number of available bytes in the receive buffer.
//! 
//! If the buffered mode is disabled then this function returns 0.
//! 
//! After calling this function the status of the configuration may be checked via the errno variable for each uart module using \ref DRV_UART_GetErrno
//! \param uart The UART module.
//! \return Number of available byte in the receive buffer
//*****************************************************************************
uint32_t DRV_UART_BytesAvailable(UART uart)
{	
	assert(uart <= UART_4);
	if (uartMaps[uart].uart_buffering_enabled == FALSE)
	{
		uartMaps[uart].uart_errno = UART_ERROR_BUFFERING_DISABLED;
		return 0;
	}
	if (!uartMaps[uart].uart_buffering_enabled)
	{
		return 0;
	}
	else
	{
		return RingBufUsed(uartMaps[uart].uart_rx_ring_buffer);
	}
}


//*****************************************************************************
//! Read a byte from selected uart using buffered mode. 
//! 
//! If the buffered mode is disabled then this function has no effect
//! 
//! After calling this function the status of the configuration may be checked via the errno variable for each uart module using \ref DRV_UART_GetErrno
//! \param uart The UART module.
//! \param dst Pointer to the memory zone where the read byte should be written.
//! \return The result of the operation. See \ref STATUS
//*****************************************************************************
STATUS DRV_UART_ReadByte(UART uart, uint8_t *dst)
{
	assert(uart <= UART_4);
	if (uartMaps[uart].uart_buffering_enabled == FALSE)
	{
		uartMaps[uart].uart_errno = UART_ERROR_BUFFERING_DISABLED;
		return NOT_OK;
	}

	TIMER_SOFTWARE_reset_timer(uartMaps[uart].uart_timer);
	TIMER_SOFTWARE_start_timer(uartMaps[uart].uart_timer);
	while (RingBufEmpty(uartMaps[uart].uart_rx_ring_buffer))
	{
		if (TIMER_SOFTWARE_interrupt_pending(uartMaps[uart].uart_timer))
		{
			TIMER_SOFTWARE_clear_interrupt(uartMaps[uart].uart_timer);
			TIMER_SOFTWARE_stop_timer(uartMaps[uart].uart_timer);
			uartMaps[uart].uart_errno = UART_ERROR_READ_TIMEOUT;
			return TIMEOUT;
		}
	}
	*dst = RingBufReadOne(uartMaps[uart].uart_rx_ring_buffer);
	uartMaps[uart].uart_errno = UART_ERROR_NO_ERROR;
	return OK;
}

//*****************************************************************************
//! Send a byte through selected uart using buffered mode. 
//! 
//! If the buffered mode is disabled then this function has no effect
//! 
//! After calling this function the status of the configuration may be checked via the errno variable for each uart module using \ref DRV_UART_GetErrno
//! \param uart The UART module.
//! \param byte The data to be sent.
//! \return The result of the operation. See \ref STATUS
//*****************************************************************************
STATUS DRV_UART_SendByte(UART uart, uint8_t byte)
{
	assert(uart <= UART_4);
	if (uartMaps[uart].uart_buffering_enabled == FALSE)
	{
		uartMaps[uart].uart_errno = UART_ERROR_BUFFERING_DISABLED;
		return NOT_OK;
	}
	TIMER_SOFTWARE_reset_timer(uartMaps[uart].uart_timer);
	TIMER_SOFTWARE_start_timer(uartMaps[uart].uart_timer);
	while(RingBufFull(uartMaps[uart].uart_tx_ring_buffer))
	{
		if (TIMER_SOFTWARE_interrupt_pending(uartMaps[uart].uart_timer))
		{
			TIMER_SOFTWARE_stop_timer(uartMaps[uart].uart_timer);
			TIMER_SOFTWARE_clear_interrupt(uartMaps[uart].uart_timer);
			return TIMEOUT;
		}
	}
	DRV_UART_PrivateSendByte(uart, byte);
	return OK;
}

//*****************************************************************************
//! Gets the current error status of the selected uart module.
//! 
//! \param uart The UART module.
//! \return Returns the error status. See \ref UART_ERROR_TYPE for possible values.
//*****************************************************************************
UART_ERROR_TYPE DRV_UART_GetErrno(UART uart)
{
	assert(uart <= UART_4);
	return uartMaps[uart].uart_errno;
}

//*****************************************************************************
//! Read a buffer from selected uart using buffered mode. 
//! The functions reads \b count bytes from the uart module and returns the amount of bytes that were received. If the read write timeout expires before the function could read \b count bytes, then the function return the number of bytes that were received before the timeout expires.
//!
//! If the buffered mode is disabled then this function has no effect.
//! 
//! After calling this function the status of the configuration may be checked via the errno variable for each uart module using \ref DRV_UART_GetErrno
//! \param uart The UART module.
//! \param dest Pointer to the memory zone where the read bytes should be placed.
//! \param count The number of bytes to be read.
//! \return The amount of bytes that were received and written at the memory zone designated by \b dest.
//*****************************************************************************
uint32_t DRV_UART_Read(UART uart, uint8_t *dest, uint32_t count)
{
	uint32_t countExisting = 0;
	assert(uart <= UART_4);
	if (uartMaps[uart].uart_buffering_enabled == FALSE)
	{
		uartMaps[uart].uart_errno = UART_ERROR_BUFFERING_DISABLED;
		return 0;
	}
	if (count > uartMaps[uart].uart_rx_buffer_size)
	{
		uartMaps[uart].uart_errno = UART_ERROR_BUFFER_LIMITS_EXCEEDED;
		return 0;
	}
	countExisting = RingBufUsed(uartMaps[uart].uart_rx_ring_buffer);
	if (count <= countExisting)
	{
		// there are enough data in the ringbuffer to return
		RingBufRead(uartMaps[uart].uart_rx_ring_buffer, dest, count);
		uartMaps[uart].uart_errno = UART_ERROR_NO_ERROR;
		return count;
	}
	else
	{
		TIMER_SOFTWARE_reset_timer(uartMaps[uart].uart_timer);
		TIMER_SOFTWARE_start_timer(uartMaps[uart].uart_timer);		
		while (countExisting < count)
		{	
			if (TIMER_SOFTWARE_interrupt_pending(uartMaps[uart].uart_timer))
			{
				TIMER_SOFTWARE_stop_timer(uartMaps[uart].uart_timer);
				TIMER_SOFTWARE_clear_interrupt(uartMaps[uart].uart_timer);
				RingBufRead(uartMaps[uart].uart_rx_ring_buffer, dest, countExisting);
				uartMaps[uart].uart_errno = UART_ERROR_READ_TIMEOUT;
				return countExisting;
			}			
			countExisting = RingBufUsed(uartMaps[uart].uart_rx_ring_buffer);
		}
		RingBufRead(uartMaps[uart].uart_rx_ring_buffer, dest, countExisting);
		uartMaps[uart].uart_errno = UART_ERROR_NO_ERROR;	
		return countExisting;
	}
}

//*****************************************************************************
//! Send a buffer through selected uart using buffered mode. 
//! The functions write \b count bytes to the uart module and returns the amount of bytes that were transmitted. If the read write timeout expires before the function could send \b count bytes, then the function returns the number of bytes that were transmitted before the timeout expires.
//!
//! If the buffered mode is disabled then this function has no effect.
//! 
//! After calling this function the status of the configuration may be checked via the errno variable for each uart module using \ref DRV_UART_GetErrno
//! \param uart The UART module.
//! \param src Pointer to the buffer that is to be sent.
//! \param count The number of bytes to send from the buffer designated by \b src.
//! \return The amount of bytes that were successfuly transmitted.
//*****************************************************************************
uint32_t DRV_UART_Write(UART uart, uint8_t *src, uint32_t count)
{
	uint32_t countFree = 0;
	uint32_t i = 0;
	assert(uart <= UART_4);	
	if (uartMaps[uart].uart_buffering_enabled == FALSE)
	{
		uartMaps[uart].uart_errno = UART_ERROR_BUFFERING_DISABLED;
		return 0;
	}
	if (count > uartMaps[uart].uart_tx_buffer_size)
	{
		uartMaps[uart].uart_errno = UART_ERROR_BUFFER_LIMITS_EXCEEDED;
		return 0;
	}
	countFree = RingBufFree(uartMaps[uart].uart_tx_ring_buffer);
	if (count <= countFree)
	{
		for (i = 0; i < count; i++)
		{
			DRV_UART_PrivateSendByte(uart, src[i]);
		}
		return count;
	}
	else
	{
		while (countFree < count)
		{	
			if (TIMER_SOFTWARE_interrupt_pending(uartMaps[uart].uart_timer))
			{
				TIMER_SOFTWARE_stop_timer(uartMaps[uart].uart_timer);
				TIMER_SOFTWARE_clear_interrupt(uartMaps[uart].uart_timer);
				for (i = 0; i < countFree; i++)
				{
					DRV_UART_PrivateSendByte(uart, src[i]);
				}
				
				uartMaps[uart].uart_errno = UART_ERROR_WRITE_TIMEOUT;
				return countFree;
			}			
			countFree = RingBufFree(uartMaps[uart].uart_tx_ring_buffer);
		}
		for (i = 0; i < count; i++)
		{
			DRV_UART_PrivateSendByte(uart, src[i]);
		}
		uartMaps[uart].uart_errno = UART_ERROR_NO_ERROR;
		return count;
	}
}

//*****************************************************************************
//! Uart driver main process function. This function is MANDATORY to be called in the program's main loop
//!
//! \return None.
//*****************************************************************************
void DRV_UART_Process()
{
	uint8_t i = 0;
	for (i = 0; i <= UART_4; i++)
	{
		if (uartMaps[i].isOpen)
		{
			if (uartMaps[i].uart_rx_callback != NULL)
			{
				if (uartMaps[i].trigger == TRUE)
				{
					uartMaps[i].trigger = FALSE;
					(uartMaps[i].uart_rx_callback)((UART)i, RingBufUsed(uartMaps[i].uart_rx_ring_buffer));
				}
			}
		}
	}
}

//*****************************************************************************
//! Set the receive callback function
//! 
//! If the buffered mode is disabled then this function has no effect
//! The receive user callback function is called by the driver when the receive trigger has be reached. If the trigger is not used the the function is called when a character has been received
//! \param uart_port The UART module.
//! \param callback The user RX Callback function.
//! \return The result of the operation. See \ref STATUS
//*****************************************************************************
STATUS DRV_UART_SetRxCallback(UART uart_port, UART_RX_CALLBACK callback)
{
	if (uart_port > UART_4)
	{
		return NOT_OK;
	}
	uartMaps[uart_port].uart_rx_callback = callback;
	return OK;
}

//*****************************************************************************
//! Set the transmission callback function
//! 
//! If the buffered mode is disabled then this function has no effect
//! 
//! \param uart_port The UART module.
//! \param callback The user TX Callback function.
//! \return The result of the operation. See \ref STATUS
//*****************************************************************************
STATUS DRV_UART_SetTxCallback(UART uart_port, UART_TX_CALLBACK callback)
{
	if (uart_port > UART_4)
	{
		return NOT_OK;
	}
	uartMaps[uart_port].uart_tx_callback = callback;
	return OK;
}

//*****************************************************************************
//! Set the receive trigger function
//! 
//! If the buffered mode is disabled then this function has no effect
//! The trigger function designates when the RX callback should be called. 
//! \param uart_port The UART module.
//! \param function The user RX Callback function.
//! \return The result of the operation. See \ref STATUS
//*****************************************************************************
STATUS DRV_UART_SetRxTriggerFunction(UART uart_port, UART_RX_TRIGGER function)
{
	if (uart_port > UART_4)
	{
		return NOT_OK;
	}
	uartMaps[uart_port].uart_rx_trigger = function;
	return OK;
}

//*****************************************************************************
//! Empty the UART reception internal buffer
//! 
//! \param uart_port The UART module.
//! \return The result of the operation. See \ref STATUS
//*****************************************************************************
STATUS DRV_UART_FlushRX(UART uart_port)
{
	RingBufFlush(uartMaps[uart_port].uart_rx_ring_buffer);
	return OK;
}

//*****************************************************************************
//! Empty the UART transmission internal buffer
//! 
//! \param uart_port The UART module.
//! \return The result of the operation. See \ref STATUS
//*****************************************************************************
STATUS DRV_UART_FlushTX(UART uart_port)
{
	RingBufFlush(uartMaps[uart_port].uart_tx_ring_buffer);
	return OK;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

