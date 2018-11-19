#include "wiznet.h"
#include "wiznet_io.h"
#include "wiznet_print.h"
#include "drv_general.h"
#include <stdint.h>
#include <Utils/timer_software.h>
#include <stdio.h>

WIZNET_SOCKET_CALLBACKS WIZNET_Socket_Callback_Array[WIZNET_SOCKET_COUNT];
WIZNET_CallBack *IPCONFLICT_Callback;
WIZNET_CallBack *PPPOECLOSE_Callback;
volatile uint8_t wiznet_interrupt_flag = 0;

timer_software_handler_t keep_alive_timeout[WIZNET_SOCKET_COUNT];
BOOLEAN socket_keep_alive_flag[WIZNET_SOCKET_COUNT];

uint16_t WIZNET_SOCKET_ADD_MAP[] = 	{
										WIZ_SOCKET_0_ADD, 
										WIZ_SOCKET_1_ADD, 
										WIZ_SOCKET_2_ADD, 
										WIZ_SOCKET_3_ADD,
										WIZ_SOCKET_4_ADD,
										WIZ_SOCKET_5_ADD,
										WIZ_SOCKET_6_ADD,
										WIZ_SOCKET_7_ADD,
									};

WIZNET_SOCKET_MEMORY_MAP wiznet_socket_memory_map[] =	{
															{SOCKET_0, WIZNET_MEM_1KB, WIZNET_MEM_1KB},
															{SOCKET_1, WIZNET_MEM_2KB, WIZNET_MEM_2KB},
															{SOCKET_2, WIZNET_MEM_2KB, WIZNET_MEM_2KB},
															{SOCKET_3, WIZNET_MEM_2KB, WIZNET_MEM_2KB},
															{SOCKET_4, WIZNET_MEM_2KB, WIZNET_MEM_2KB},
															{SOCKET_5, WIZNET_MEM_2KB, WIZNET_MEM_2KB},
															{SOCKET_6, WIZNET_MEM_2KB, WIZNET_MEM_2KB},
															{SOCKET_7, WIZNET_MEM_2KB, WIZNET_MEM_2KB},
														};

void keep_alive_timer_callback(timer_software_handler_t h)
{
	uint8_t i = 0;
	WIZNET_SOCKETS socket;
	BOOLEAN found = FALSE;
	for (i = 0; i < WIZNET_SOCKET_COUNT; i++)
	{
		if (h == keep_alive_timeout[i])
		{
			found = TRUE;
			socket = (WIZNET_SOCKETS)i;
			break;
		}
	}
	if (found == FALSE)
	{
		return;
	}
	socket_keep_alive_flag[socket] = TRUE;
}


uint16_t WIZNET_CalculateRxSocketOffset(WIZNET_SOCKETS socket)
{
	uint16_t result = 0;
	uint16_t i = 0;
	for (i = 0; i < socket; i++)
	{
		result += (wiznet_socket_memory_map[i].rx_mem_size);
	}
	return result;
}

uint16_t WIZNET_CalculateTxSocketOffset(WIZNET_SOCKETS socket)
{
	uint16_t result = 0;
	uint16_t i = 0;
	for (i = 0; i < socket; i++)
	{
		result += (wiznet_socket_memory_map[i].tx_mem_size);
	}
	return result;
}

void WIZNET_InitStructures()
{
	uint8_t i;
	for (i = 0; i < WIZNET_SOCKET_COUNT; i++)
	{
		WIZNET_Socket_Callback_Array[i].CON = NULL;
		WIZNET_Socket_Callback_Array[i].DISCON = NULL;
		WIZNET_Socket_Callback_Array[i].PFAIL = NULL;
		WIZNET_Socket_Callback_Array[i].PNEXT = NULL;
		WIZNET_Socket_Callback_Array[i].PRECV = NULL;
		WIZNET_Socket_Callback_Array[i].RECV = NULL;
		WIZNET_Socket_Callback_Array[i].SEND_OK = NULL;
		WIZNET_Socket_Callback_Array[i].TIMEOUT = NULL;
		socket_keep_alive_flag[i] = FALSE;
		keep_alive_timeout[i] = -1;
	}
	IPCONFLICT_Callback = NULL;
	PPPOECLOSE_Callback = NULL;
}

void WIZNET_SOCKET_ISR(WIZNET_SOCKETS socket)
{
	uint8_t sock_ir = WIZNET_GetSocketIR(socket);
	if ((sock_ir & (1 << 7)) != 0)
	{
		// PRECV
		if (WIZNET_Socket_Callback_Array[socket].PRECV != NULL)
		{
			(WIZNET_Socket_Callback_Array[socket].PRECV)();
		}
	}
	if ((sock_ir & (1 << 6)) != 0)
	{
		//PFAIL
		if (WIZNET_Socket_Callback_Array[socket].PFAIL != NULL)
		{
			(WIZNET_Socket_Callback_Array[socket].PFAIL)();
		}
	}
	if ((sock_ir & (1 << 5)) != 0)
	{
		//PNEXT
		if (WIZNET_Socket_Callback_Array[socket].PNEXT != NULL)
		{
			(WIZNET_Socket_Callback_Array[socket].PNEXT)();
		}
	}
	if ((sock_ir & (1 << 4)) != 0)
	{
		//SEND_OK
		if (WIZNET_Socket_Callback_Array[socket].SEND_OK != NULL)
		{
			(WIZNET_Socket_Callback_Array[socket].SEND_OK)();
		}
	}
	if ((sock_ir & (1 << 3)) != 0)
	{
		//TIMEOUT
		TIMER_SOFTWARE_stop_timer(keep_alive_timeout[socket]);
		TIMER_SOFTWARE_reset_timer(keep_alive_timeout[socket]);
		if (WIZNET_Socket_Callback_Array[socket].TIMEOUT != NULL)
		{
			(WIZNET_Socket_Callback_Array[socket].TIMEOUT)();
		}
	}
	if ((sock_ir & (1 << 2)) != 0)
	{
		//RECV
		TIMER_SOFTWARE_reset_timer(keep_alive_timeout[socket]);
		if (WIZNET_Socket_Callback_Array[socket].RECV != NULL)
		{
			(WIZNET_Socket_Callback_Array[socket].RECV)();
		}
	}
	if ((sock_ir & (1 << 1)) != 0)
	{
		//DISCON
		TIMER_SOFTWARE_stop_timer(keep_alive_timeout[socket]);
		if (WIZNET_Socket_Callback_Array[socket].DISCON != NULL)
		{
			(WIZNET_Socket_Callback_Array[socket].DISCON)();
		}
	}
	if ((sock_ir & (1 << 0)) != 0)
	{
		//CON
		TIMER_SOFTWARE_reset_timer(keep_alive_timeout[socket]);
		TIMER_SOFTWARE_start_timer(keep_alive_timeout[socket]);
		if (WIZNET_Socket_Callback_Array[socket].CON != NULL)
		{
			(WIZNET_Socket_Callback_Array[socket].CON)();
		}
	}
	WIZNET_SetSocketIR(socket, sock_ir);
}

void WIZNET_ISR()
{
	uint8_t ir2 = WIZNET_GetIR2();
	uint8_t ir = WIZNET_GetIR();
	uint8_t i;
	if ((ir & (1 << 7)) != 0)
	{
		// ip conflict
		if (IPCONFLICT_Callback != NULL)
		{
			(IPCONFLICT_Callback)();
		}
		ir |= 1 << 7;
		WIZNET_SetIR(ir);
	}
	if ((ir & (1 << 5)) != 0)
	{
		// pppoe connection close 
		if (PPPOECLOSE_Callback != NULL)
		{
			(PPPOECLOSE_Callback)();
		}
		ir |= 1 << 5;
		WIZNET_SetIR(ir);
	}
	for (i = 0; i < WIZNET_SOCKET_COUNT; i++)
	{
		if ((ir2 & (1 << i)) != 0)
		{
			WIZNET_SOCKET_ISR((WIZNET_SOCKETS)i);
		}
	}
}

STATUS WIZNET_Reset()
{
	timer_software_handler_t timer;
	timer = TIMER_SOFTWARE_request_timer();
	if (timer == -1)
	{
		return NO_TIMERS_AVAILABLE;
	}
	TIMER_SOFTWARE_configure_timer(timer, MODE_0, 2, 1);
	
	WIZNET_ResetLo();
	TIMER_SOFTWARE_start_timer(timer);
	while (!TIMER_SOFTWARE_interrupt_pending(timer));
	WIZNET_ResetHi();
	TIMER_SOFTWARE_clear_interrupt(timer);
	TIMER_SOFTWARE_reset_timer(timer);
	TIMER_SOFTWARE_configure_timer(timer, MODE_0, 150, 1);
	TIMER_SOFTWARE_start_timer(timer);
	while (!TIMER_SOFTWARE_interrupt_pending(timer));
	TIMER_SOFTWARE_release_timer(timer);
	return OK;
}

STATUS WIZNET_Module_Init()
{
	STATUS status;
	WIZNET_InitStructures();
	WIZNET_Init_IO();
	status = WIZNET_Reset();
	if (status != OK)
	{
		return status;
	}
	WIZNET_InitInterrupt();
	WIZNET_SetWiznetMemory();
	return OK;
}

uint8_t WIZNET_GetChipVersion()
{
	uint8_t version = 0;
	uint16_t lengthop;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((WIZ_REG_CHIP_VERSION >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((WIZ_REG_CHIP_VERSION >> 0) & 0xFF);	
	lengthop = 1;
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	version = WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return version;
}

WIZNET_PHY_STATUS WIZNET_GetPhyStatus()
{
	uint8_t stat = 0;
	WIZNET_PHY_STATUS status = {LINK_DOWN, POWERDOWN_DISABLE};
	uint16_t lengthop;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((WIZ_REG_PHYSTATUS >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((WIZ_REG_PHYSTATUS >> 0) & 0xFF);	
	lengthop = 1;
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	stat = WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	if (stat & (1 << 5))
	{
		status.link_status = LINK_UP;
	}
	if (stat & (1 << 3))
	{
		status.powerdown_status = POWERDOWN_ENABLE;
	}
	return status;	
}

void WIZNET_SetIMR(uint8_t value)
{
	uint16_t lengthop;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((WIZ_REG_IMR >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((WIZ_REG_IMR >> 0) & 0xFF);
	lengthop = 1 | (1 << 15);	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive(value);
	WIZNET_Deselect();
}

uint8_t WIZNET_GetIMR()
{
	uint8_t data;
	uint16_t lengthop;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((WIZ_REG_IMR >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((WIZ_REG_IMR >> 0) & 0xFF);
	lengthop = 1;	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	data = WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return data;
}


void WIZNET_SetIMR2(uint8_t value)
{
	uint16_t lengthop;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((WIZ_REG_IMR2 >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((WIZ_REG_IMR2 >> 0) & 0xFF);
	lengthop = 1 | (1 << 15);	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive(value);
	WIZNET_Deselect();
}

uint8_t WIZNET_GetIMR2()
{
	uint8_t data;
	uint16_t lengthop;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((WIZ_REG_IMR2 >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((WIZ_REG_IMR2 >> 0) & 0xFF);
	lengthop = 1;	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	data = WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return data;
}

void WIZNET_SetIR2(uint8_t value)
{
	uint16_t lengthop;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((WIZ_REG_IR2 >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((WIZ_REG_IR2 >> 0) & 0xFF);
	lengthop = 1 | (1 << 15);	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive(value);
	WIZNET_Deselect();
}

uint8_t WIZNET_GetIR2()
{
	uint8_t data;
	uint16_t lengthop;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((WIZ_REG_IR2 >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((WIZ_REG_IR2 >> 0) & 0xFF);
	lengthop = 1;	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	data = WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return data;
}

void WIZNET_SetIR(uint8_t value)
{
	uint16_t lengthop;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((WIZ_REG_IR >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((WIZ_REG_IR >> 0) & 0xFF);
	lengthop = 1 | (1 << 15);	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive(value);
	WIZNET_Deselect();
}

uint8_t WIZNET_GetIR()
{
	uint8_t data;
	uint16_t lengthop;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((WIZ_REG_IR >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((WIZ_REG_IR >> 0) & 0xFF);
	lengthop = 1;	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	data = WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return data;
}

uint8_t WIZNET_GetSocketIR(WIZNET_SOCKETS socket)
{
	uint8_t data;
	uint16_t lengthop;
	uint16_t register_address = WIZ_REG_Sn_IR + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 1;	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	data = WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return data;
}

void WIZNET_SetSocketIR(WIZNET_SOCKETS socket, uint8_t value)
{
	uint16_t lengthop;
	uint16_t register_address = WIZ_REG_Sn_IR + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 1 | (1 << 15);	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive(value);
	WIZNET_Deselect();
}

uint8_t WIZNET_GetSocketIMR(WIZNET_SOCKETS socket)
{
	uint8_t data;
	uint16_t lengthop;
	uint16_t register_address = WIZ_REG_Sn_IMR + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 1;	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	data = WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return data;
}

void WIZNET_SetSocketIMR(WIZNET_SOCKETS socket, uint8_t value)
{
	uint16_t lengthop;
	uint16_t register_address = WIZ_REG_Sn_IMR + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 1 | (1 << 15);	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive(value);
	WIZNET_Deselect();
}

void WIZNET_SetGeneralPort(uint16_t register_address, uint16_t port)
{
	uint16_t lengthop;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 2 | (1 << 15);
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive((port >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((port >> 0) & 0xFF);
	WIZNET_Deselect();	
}

uint16_t WIZNET_GetGeneralPort(uint16_t register_address)
{
	uint16_t port;
	uint16_t lengthop = 2;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);	
	port  = WIZNET_SPI_SendReceive(0xFF) << 8;
	port |= WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return port;
}


void WIZNET_SetGeneralIp(uint16_t register_address, WIZNET_IP_ADDRESS ip_address)
{
	uint16_t lengthop;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 4 | (1 << 15);
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive(ip_address.ip1);
	WIZNET_SPI_SendReceive(ip_address.ip2);
	WIZNET_SPI_SendReceive(ip_address.ip3);
	WIZNET_SPI_SendReceive(ip_address.ip4);	
	WIZNET_Deselect();
}

void WIZNET_SetGeneralHwAddress(uint16_t register_address, WIZNET_HW_ADDRESS hw_address)
{
	uint16_t lengthop;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 6 | (1 << 15);
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);	
	WIZNET_SPI_SendReceive(hw_address.hw1);
	WIZNET_SPI_SendReceive(hw_address.hw2);
	WIZNET_SPI_SendReceive(hw_address.hw3);
	WIZNET_SPI_SendReceive(hw_address.hw4);
	WIZNET_SPI_SendReceive(hw_address.hw5);
	WIZNET_SPI_SendReceive(hw_address.hw6);
	WIZNET_Deselect();
}

WIZNET_IP_ADDRESS WIZNET_GetGeneralIp(uint16_t register_address)
{
	WIZNET_IP_ADDRESS address;
	uint16_t lengthop = 4;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);	
	address.ip1 = WIZNET_SPI_SendReceive(0xFF);
	address.ip2 = WIZNET_SPI_SendReceive(0xFF);
	address.ip3 = WIZNET_SPI_SendReceive(0xFF);
	address.ip4 = WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return address;
}

WIZNET_HW_ADDRESS WIZNET_GetGeneralHwAddress(uint16_t register_address)
{
	WIZNET_HW_ADDRESS address;
	uint16_t lengthop = 6;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);	
	address.hw1 = WIZNET_SPI_SendReceive(0xFF);
	address.hw2 = WIZNET_SPI_SendReceive(0xFF);
	address.hw3 = WIZNET_SPI_SendReceive(0xFF);
	address.hw4 = WIZNET_SPI_SendReceive(0xFF);
	address.hw5 = WIZNET_SPI_SendReceive(0xFF);
	address.hw6 = WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return address;
}


WIZNET_IP_ADDRESS WIZNET_GetIpAddress()
{
	return WIZNET_GetGeneralIp(WIZ_REG_SIPR);
}
	
WIZNET_IP_ADDRESS WIZNET_GetSubnetMask()
{
	return WIZNET_GetGeneralIp(WIZ_REG_SUBR);
}

void WIZNET_SetIpAddress(WIZNET_IP_ADDRESS ip_address)
{
	WIZNET_SetGeneralIp(WIZ_REG_SIPR, ip_address);
}

void WIZNET_SetSubnetMask(WIZNET_IP_ADDRESS ip_address)
{
	WIZNET_SetGeneralIp(WIZ_REG_SUBR, ip_address);
}

void WIZNET_SetGateway(WIZNET_IP_ADDRESS ip_address)
{
	WIZNET_SetGeneralIp(WIZ_REG_GAR, ip_address);
}

WIZNET_IP_ADDRESS WIZNET_GetGateway()
{
	return WIZNET_GetGeneralIp(WIZ_REG_GAR);
}

void WIZNET_SetHardwareAddress(WIZNET_HW_ADDRESS hw_address)
{
	WIZNET_SetGeneralHwAddress(WIZ_REG_SHAR, hw_address);
}

WIZNET_HW_ADDRESS WIZNET_GetHardwareAddress()
{
	return WIZNET_GetGeneralHwAddress(WIZ_REG_SHAR);
}

STATUS WIZNET_SetGeneralInterrupt(WIZNET_GENERAL_INTERRUPT_SOURCES intr_source, WIZNET_CallBack *callback)
{
	uint8_t imr2 = 0x00;
	imr2 = WIZNET_GetIMR2();
	switch (intr_source)
	{
		case WIZNET_IPCONFLICT_IMR:
		{
			imr2 |= 1 << 7;
			WIZNET_SetIMR2(imr2);
			IPCONFLICT_Callback = callback;
			break;
		}
		case WIZNET_PPPOECLOSE_IMR:
		{
			imr2 |= 1 << 5;
			WIZNET_SetIMR2(imr2);
			PPPOECLOSE_Callback = callback;
			break;
		}
		default:
		{
			return INCORRECT_PARAMS;
		}
	}
	return OK;
}

STATUS WIZNET_ClrGeneralInterrupt(WIZNET_GENERAL_INTERRUPT_SOURCES intr_source)
{
	uint8_t imr2 = 0x00;
	imr2 = WIZNET_GetIMR2();
	switch (intr_source)
	{
		case WIZNET_PPPOECLOSE_IMR:
		{
			imr2 &= ~(1 << 7);
			WIZNET_SetIMR2(imr2);
			break;
		}
		case WIZNET_IPCONFLICT_IMR:
		{
			imr2 &= ~(1 << 5);
			WIZNET_SetIMR2(imr2);
			break;
		}
		default:
		{
			return INCORRECT_PARAMS;
		}
	}
	return OK;
}

STATUS WIZNET_SetSocketInterrupt(WIZNET_SOCKETS socket, WIZNET_SOCKET_INTERRUPT_SOURCES intr_source, WIZNET_CallBack *callback)
{
	uint8_t sock_imr = 0x00;
	uint8_t imr = 0x00;
	sock_imr = WIZNET_GetSocketIMR(socket);
	imr = WIZNET_GetIMR();
	switch (intr_source)
	{
		case WIZNET_SOCK_INTR_CON:
		{
			WIZNET_Socket_Callback_Array[socket].CON = callback;
			break;
		}
		case WIZNET_SOCK_INTR_DISCON:
		{
			WIZNET_Socket_Callback_Array[socket].DISCON = callback;
			break;
		}
		case WIZNET_SOCK_INTR_RECV:
		{
			WIZNET_Socket_Callback_Array[socket].RECV = callback;
			break;
		}
		case WIZNET_SOCK_INTR_TIMEOUT:
		{
			WIZNET_Socket_Callback_Array[socket].TIMEOUT = callback;
			break;
		}
		case WIZNET_SOCK_INTR_SENDOK:
		{
			WIZNET_Socket_Callback_Array[socket].SEND_OK = callback;
			break;
		}
		case WIZNET_SOCK_INTR_PNEXT:
		{
			WIZNET_Socket_Callback_Array[socket].PNEXT = callback;
			break;
		}
		case WIZNET_SOCK_INTR_PFAIL:
		{
			WIZNET_Socket_Callback_Array[socket].PFAIL = callback;
			break;
		}
		case WIZNET_SOCK_INTR_PRECV:		
		{
			WIZNET_Socket_Callback_Array[socket].PRECV = callback;
			break;
		}
		default:
		{
			return INCORRECT_PARAMS;
		}
	}	
	sock_imr |= 1 << intr_source;
	WIZNET_SetSocketIMR(socket, sock_imr);
	imr |= 1 << socket;
	WIZNET_SetIMR(imr);
	return OK;
}

STATUS WIZNET_ClrSocketInterrupt(WIZNET_SOCKETS socket, WIZNET_SOCKET_INTERRUPT_SOURCES intr_source)
{
	uint8_t sock_imr = 0x00;
	sock_imr = WIZNET_GetSocketIMR(socket);
	sock_imr &= ~(1 << intr_source);
	WIZNET_SetSocketIMR(socket, sock_imr);
	return OK;	
}

STATUS WIZNET_ClrAllSocketInterrupt(WIZNET_SOCKETS socket)
{
	uint8_t imr = 0x00;
	imr = WIZNET_GetIMR();
	WIZNET_SetSocketIMR(socket, 0x00);
	imr &= ~(1 << socket);
	WIZNET_SetIMR(imr);
	return OK;	
}

WIZNET_SOCKET_STATUS WIZNET_GetSocketStatus(WIZNET_SOCKETS socket)
{
	uint8_t data;
	uint16_t lengthop;
	uint16_t register_address = WIZ_REG_Sn_SR + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 1;	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	data = WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return (WIZNET_SOCKET_STATUS)data;	
}

void WIZNET_SendSocketCommand(WIZNET_SOCKETS socket, WIZNET_SOCKET_COMMAND command)
{
	uint16_t lengthop;
	uint16_t register_address = WIZ_REG_Sn_CR + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 1 | (1 << 15);;
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive(command);
	WIZNET_Deselect();
}

void WIZNET_SetModeRegister(WIZNET_SOCKETS socket, uint8_t value)
{
	uint16_t lengthop;
	uint16_t register_address = WIZ_REG_Sn_MR + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 1 | (1 << 15);
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive(value);
	WIZNET_Deselect();
}

uint8_t WIZNET_GetModeRegister(WIZNET_SOCKETS socket)
{
	uint16_t lengthop;
	uint8_t value;
	uint16_t register_address = WIZ_REG_Sn_MR + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 1;
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	value = WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return value;
}

void WIZNET_SetSocketSourcePort(WIZNET_SOCKETS socket, uint16_t port)
{
	WIZNET_SetGeneralPort(WIZ_REG_Sn_PORT + WIZNET_SOCKET_ADD_MAP[socket], port);
}

uint16_t WIZNET_GetSocketSourcePort(WIZNET_SOCKETS socket)
{
	return WIZNET_GetGeneralPort(WIZ_REG_Sn_PORT + WIZNET_SOCKET_ADD_MAP[socket]);
}

WIZNET_HW_ADDRESS WIZNET_GetSocketDestinationHwAddress(WIZNET_SOCKETS socket)
{
	return WIZNET_GetGeneralHwAddress(WIZ_REG_Sn_DHAR + WIZNET_SOCKET_ADD_MAP[socket]);
}

void WIZNET_SetSocketDestinationHwAddress(WIZNET_SOCKETS socket, WIZNET_HW_ADDRESS hw_address)
{
	WIZNET_SetGeneralHwAddress(WIZ_REG_Sn_DHAR + WIZNET_SOCKET_ADD_MAP[socket], hw_address);
}

WIZNET_IP_ADDRESS WIZNET_GetSocketDestinationIpAddress(WIZNET_SOCKETS socket)
{
	return WIZNET_GetGeneralIp(WIZ_REG_Sn_DIPR + WIZNET_SOCKET_ADD_MAP[socket]);
}

void WIZNET_SetSocketDestinationIpAddress(WIZNET_SOCKETS socket, WIZNET_IP_ADDRESS ip_address)
{
	WIZNET_SetGeneralIp(WIZ_REG_Sn_DIPR + WIZNET_SOCKET_ADD_MAP[socket], ip_address);
}

void WIZNET_InitSocket(WIZNET_SOCKETS socket)
{
	WIZNET_SendSocketCommand(socket, SOCKET_OPEN);
	while (WIZNET_GetSocketStatus(socket) != SOCK_INIT);
}

void WIZNET_ListenSocket(WIZNET_SOCKETS socket)
{
	WIZNET_SendSocketCommand(socket, SOCKET_LISTEN);
	while (WIZNET_GetSocketStatus(socket) != SOCK_LISTEN);

}

STATUS WIZNET_ConfigureSocket_TCPServer(WIZNET_SOCKETS socket, uint16_t port, uint32_t keep_alive_period)
{
    TIMER_SOFTWARE_release_timer(keep_alive_timeout[socket]);
	keep_alive_timeout[socket] = TIMER_SOFTWARE_request_timer();
	TIMER_SOFTWARE_configure_timer(keep_alive_timeout[socket], MODE_1, keep_alive_period, 1);	
	TIMER_SOFTWARE_set_callback(keep_alive_timeout[socket], keep_alive_timer_callback);	
	
	WIZNET_SetModeRegister(socket, 0x01);
	WIZNET_SetSocketSourcePort(socket, port);
	return OK;
}

STATUS WIZNET_OpenSocket_TCPServer(WIZNET_SOCKETS socket)
{
	WIZNET_InitSocket(socket);
	WIZNET_ListenSocket(socket);
	TIMER_SOFTWARE_reset_timer(keep_alive_timeout[socket]);
	
	return OK;
}

uint16_t WIZNET_GetReceivedSize(WIZNET_SOCKETS socket)
{
	uint16_t lengthop;
	uint16_t value;
	uint16_t register_address = WIZ_REG_Sn_RSR + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 2;
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	value = (uint16_t)WIZNET_SPI_SendReceive(0xFF) << 8;
	value |= WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return value;	
}

uint16_t WIZNET_GetRxReadPointer(WIZNET_SOCKETS socket)
{
	uint16_t lengthop;
	uint16_t value;
	uint16_t register_address = WIZ_REG_Sn_RD + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 2;
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	value = (uint16_t)WIZNET_SPI_SendReceive(0xFF) << 8;
	value |= WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return value;
}

uint16_t WIZNET_GetSocketTxFreeSize(WIZNET_SOCKETS socket)
{
	uint16_t lengthop;
	uint16_t value;
	uint16_t register_address = WIZ_REG_Sn_TX_FSR + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 2;
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	value = (uint16_t)WIZNET_SPI_SendReceive(0xFF) << 8;
	value |= WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return value;
}

uint16_t WIZNET_GetSocketTxWritePointer(WIZNET_SOCKETS socket)
{
	uint16_t lengthop;
	uint16_t value;
	uint16_t register_address = WIZ_REG_Sn_RX_WR + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 2;
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	value = (uint16_t)WIZNET_SPI_SendReceive(0xFF) << 8;
	value |= WIZNET_SPI_SendReceive(0xFF);
	WIZNET_Deselect();
	return value;
}

void WIZNET_SetSocketTxWritePointer(WIZNET_SOCKETS socket, uint16_t value)
{
	uint16_t lengthop;
	uint16_t register_address = WIZ_REG_Sn_RX_WR + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 2 | (1 << 15);
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive((value >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((value >> 0) & 0xFF);
	WIZNET_Deselect();
}




void WIZNET_SetRxReadPointer(WIZNET_SOCKETS socket, uint16_t value)
{
	uint16_t lengthop;
	uint16_t register_address = WIZ_REG_Sn_RD + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 2 | (1 << 15);
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive((value >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((value >> 0) & 0xFF);
	WIZNET_Deselect();
}

void WIZNET_ReadRxMemory(uint16_t address, uint8_t *dest, uint16_t size)
{
	uint16_t lengthop;	
	uint16_t i;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((address >> 0) & 0xFF);
	lengthop = size;
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	for (i = 0; i < size; i++)
	{
		dest[i] = WIZNET_SPI_SendReceive(0xFF);
	}
	WIZNET_Deselect();
}

void WIZNET_WriteTxMemory(uint16_t address, uint8_t *src, uint16_t size)
{
	uint16_t lengthop;	
	uint16_t i;
	WIZNET_Select();
	WIZNET_SPI_SendReceive((address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((address >> 0) & 0xFF);
	lengthop = size | (1 << 15);
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	for (i = 0; i < size; i++)
	{
		WIZNET_SPI_SendReceive(src[i]);
	}
	WIZNET_Deselect();
	
}

uint32_t WIZNET_ReadData(WIZNET_SOCKETS socket, uint8_t *buffer)
{
	uint16_t length;
	uint16_t src_mask;
	uint16_t src_ptr;
	uint16_t upper_size;
	uint16_t left_size;
	uint16_t index = 0;
	uint16_t rx_rd = 0;
	uint16_t socket_base_address = WIZNET_CalculateRxSocketOffset(socket) + WIZNET_RX_MEMORY_BASE;
	length = WIZNET_GetReceivedSize(socket);
	rx_rd = WIZNET_GetRxReadPointer(socket);
	src_mask = rx_rd & (wiznet_socket_memory_map[socket].rx_mem_size - 1);
	src_ptr = socket_base_address + src_mask;
	if ((src_mask + length) > wiznet_socket_memory_map[socket].rx_mem_size)
	{
		upper_size = wiznet_socket_memory_map[socket].rx_mem_size - src_mask;
		WIZNET_ReadRxMemory(src_ptr, buffer, upper_size);
		index += upper_size;
		left_size = length - upper_size;
		WIZNET_ReadRxMemory(socket_base_address, &buffer[index], left_size);
		index += left_size;
	}
	else
	{
		WIZNET_ReadRxMemory(src_ptr, buffer, length);
		index += length;
	}
	
	WIZNET_SetRxReadPointer(socket, rx_rd + length);
	WIZNET_SendSocketCommand(socket, SOCKET_RECV);
	return index;
}

void WIZNET_Process()
{
	uint8_t i = 0;
	if (wiznet_interrupt_flag == 1)
	{
		while (WIZNET_InterruptPinLow())
		{
			WIZNET_ISR();
		}
		wiznet_interrupt_flag = 0;
	}
	for (i = 0; i < WIZNET_SOCKET_COUNT; i++)
	{
		if (socket_keep_alive_flag[i] == TRUE)
		{
			printf ("WIZNET: KEEP_ALIVE\n");
			socket_keep_alive_flag[i] = FALSE;
			WIZNET_SendSocketCommand((WIZNET_SOCKETS)i, SOCKET_SEND_KEEP);
		}
	}
}

STATUS WIZNET_SendData(WIZNET_SOCKETS socket, uint8_t *buffer, uint32_t size)
{
	uint16_t freesize = 0;
	uint16_t dst_mask;
	uint16_t tx_wr = 0;
	uint16_t dst_ptr = 0;
	uint16_t upper_size = 0;
	uint16_t index = 0;
	uint16_t left_size = 0;
	uint16_t socket_base_address = WIZNET_CalculateTxSocketOffset(socket) + WIZNET_TX_MEMORY_BASE;
	freesize = WIZNET_GetSocketTxFreeSize(socket);
	if (freesize < size)
	{
		return BUSY;
	}
	tx_wr = WIZNET_GetSocketTxWritePointer(socket);
	dst_mask = tx_wr & (wiznet_socket_memory_map[SOCKET_0].tx_mem_size - 1);
	dst_ptr = socket_base_address + dst_mask;
	if ((dst_mask + size) > wiznet_socket_memory_map[socket].tx_mem_size)
	{
		upper_size = wiznet_socket_memory_map[socket].tx_mem_size - dst_mask;
		WIZNET_WriteTxMemory(dst_ptr, &buffer[index], upper_size);
		index += upper_size;
		left_size = size - upper_size;
		WIZNET_WriteTxMemory(socket_base_address, &buffer[index], left_size);		
		index += left_size;
	}
	else
	{
		WIZNET_WriteTxMemory(dst_ptr, &buffer[index], size);
		index += size;
	}
	WIZNET_SetSocketTxWritePointer(socket, tx_wr + index);
	WIZNET_SendSocketCommand(socket, SOCKET_SEND);
	return OK;
}

void WIZNET_DisconnectSocket(WIZNET_SOCKETS socket)
{
	WIZNET_SendSocketCommand(SOCKET_0, SOCKET_DISCON);
	TIMER_SOFTWARE_stop_timer(keep_alive_timeout[socket]);
	TIMER_SOFTWARE_reset_timer(keep_alive_timeout[socket]);
}

void WIZNET_SetSocketRxMemSize(WIZNET_SOCKETS socket, WIZNET_MEM_SIZE_CHOICE mem_size)
{
	uint16_t lengthop;
	uint16_t register_address = WIZ_REG_Sn_RXMEM_SIZE + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 1 | (1 << 15);	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive( (uint8_t)(mem_size / 1024) );
	WIZNET_Deselect();
}

void WIZNET_SetSocketTxMemSize(WIZNET_SOCKETS socket, WIZNET_MEM_SIZE_CHOICE mem_size)
{
	uint16_t lengthop;
	uint16_t register_address = WIZ_REG_Sn_TXMEM_SIZE + WIZNET_SOCKET_ADD_MAP[socket];
	WIZNET_Select();
	WIZNET_SPI_SendReceive((register_address >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((register_address >> 0) & 0xFF);
	lengthop = 1 | (1 << 15);	
	WIZNET_SPI_SendReceive((lengthop >> 8) & 0xFF);
	WIZNET_SPI_SendReceive((lengthop >> 0) & 0xFF);
	WIZNET_SPI_SendReceive( (uint8_t)(mem_size / 1024) );
	WIZNET_Deselect();
}

void WIZNET_SetWiznetMemory()
{
	uint8_t i = 0;
	for (i = 0; i < WIZNET_SOCKET_COUNT; i++)
	{
		WIZNET_SetSocketRxMemSize((WIZNET_SOCKETS)i, wiznet_socket_memory_map[i].rx_mem_size);
		WIZNET_SetSocketTxMemSize((WIZNET_SOCKETS)i, wiznet_socket_memory_map[i].tx_mem_size);
	}
}
