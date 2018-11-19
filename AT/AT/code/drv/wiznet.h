#ifndef __WIZNET_H
#define __WIZNET_H

#include "drv_general.h"

#define WIZNET_COMMON_REGISTERS_BASE	0x0000
#define WIZNET_SOCKET_REGISTERS_BASE	0x4000
#define WIZNET_TX_MEMORY_BASE			0x8000
#define WIZNET_RX_MEMORY_BASE			0xC000

#define WIZ_REG_GAR					0x0001
#define WIZ_REG_SUBR				0x0005
#define WIZ_REG_SHAR				0x0009
#define WIZ_REG_SIPR				0x000F
#define WIZ_REG_IR					0x0015
#define WIZ_REG_IMR					0x0016
#define WIZ_REG_CHIP_VERSION		0x001F
#define WIZ_REG_IR2					0x0034
#define WIZ_REG_PHYSTATUS			0x0035
#define WIZ_REG_IMR2				0x0036

#define WIZ_SOCKET_0_ADD			0x0000
#define WIZ_SOCKET_1_ADD			0x0100
#define WIZ_SOCKET_2_ADD			0x0200
#define WIZ_SOCKET_3_ADD			0x0300
#define WIZ_SOCKET_4_ADD			0x0400
#define WIZ_SOCKET_5_ADD			0x0500
#define WIZ_SOCKET_6_ADD			0x0600
#define WIZ_SOCKET_7_ADD			0x0700

#define WIZ_REG_Sn_MR				0x4000
#define WIZ_REG_Sn_CR				0x4001
#define WIZ_REG_Sn_IR				0x4002
#define WIZ_REG_Sn_SR				0x4003
#define WIZ_REG_Sn_PORT				0x4004
#define WIZ_REG_Sn_DHAR				0x4006
#define WIZ_REG_Sn_DIPR				0x400C
#define WIZ_REG_Sn_RXMEM_SIZE		0x401E
#define WIZ_REG_Sn_TXMEM_SIZE		0x401F
#define WIZ_REG_Sn_TX_FSR			0x4020
#define WIZ_REG_Sn_RX_WR			0x4024
#define WIZ_REG_Sn_RSR				0x4026
#define WIZ_REG_Sn_RD				0x4028
#define WIZ_REG_Sn_IMR				0x402C

typedef enum
{
	LINK_DOWN,
	LINK_UP,
}WIZNET_LINK_STATUS;

typedef enum
{
	POWERDOWN_DISABLE,
	POWERDOWN_ENABLE
}WIZNET_POWERDOWN_STATUS;

typedef struct
{
	WIZNET_LINK_STATUS link_status;
	WIZNET_POWERDOWN_STATUS powerdown_status;
}WIZNET_PHY_STATUS;

typedef struct
{
	uint8_t ip1;
	uint8_t ip2;
	uint8_t ip3;
	uint8_t ip4;
}WIZNET_IP_ADDRESS;

typedef struct
{
	uint8_t hw1;
	uint8_t hw2;
	uint8_t hw3;
	uint8_t hw4;
	uint8_t hw5;
	uint8_t hw6;
}WIZNET_HW_ADDRESS;

#define WIZNET_SOCKET_COUNT 8

typedef enum
{
	SOCKET_0,
	SOCKET_1,
	SOCKET_2,
	SOCKET_3,
	SOCKET_4,
	SOCKET_5,
	SOCKET_6,
	SOCKET_7,
}WIZNET_SOCKETS;

typedef void (WIZNET_CallBack)(void);

typedef struct
{
	WIZNET_CallBack *PRECV;
	WIZNET_CallBack *PFAIL;
	WIZNET_CallBack *PNEXT;
	WIZNET_CallBack *SEND_OK;
	WIZNET_CallBack *TIMEOUT;
	WIZNET_CallBack *RECV;
	WIZNET_CallBack *DISCON;
	WIZNET_CallBack *CON;
}WIZNET_SOCKET_CALLBACKS;

typedef enum
{
	WIZNET_SOCK_INTR_CON = 0,
	WIZNET_SOCK_INTR_DISCON = 1,
	WIZNET_SOCK_INTR_RECV = 2,
	WIZNET_SOCK_INTR_TIMEOUT = 3,
	WIZNET_SOCK_INTR_SENDOK = 4,
	WIZNET_SOCK_INTR_PNEXT = 5,
	WIZNET_SOCK_INTR_PFAIL = 6,
	WIZNET_SOCK_INTR_PRECV = 7,
}WIZNET_SOCKET_INTERRUPT_SOURCES;

typedef enum
{
	WIZNET_PPPOECLOSE_IMR = 5,
	WIZNET_IPCONFLICT_IMR = 7,	
}WIZNET_GENERAL_INTERRUPT_SOURCES;

typedef enum
{
	SOCKET_OPEN = 0x01,
	SOCKET_LISTEN = 0x02,
	SOCKET_CONNECT = 0x03,
	SOCKET_DISCON = 0x08,
	SOCKET_CLOSE = 0x10,
	SOCKET_SEND = 0x20,
	SOCKET_SEND_MAC = 0x21,
	SOCKET_SEND_KEEP = 0x22,
	SOCKET_RECV = 0x40,
	SOCKET_PCON = 0x23,
	SOCKET_PDISCON = 0x24,
	SOCKET_PCR = 0x25,
	SOCKET_PCN = 0x26,
	SOCKET_PCJ = 0x27,
}WIZNET_SOCKET_COMMAND;

typedef enum
{
	SOCKET_PROTOCOL_CLOSED = 0x0,
	SOCKET_PROTOCOL_TCP = 0x1,
	SOCKET_PROTOCOL_UDP = 0x2,
	SOCKET_PROTOCOL_IPRAW = 0x3,
	SOCKET_PROTOCOL_MACRAW = 0x4,
	SOCKET_PROTOCOL_PPPOE = 0x5,
}WIZNET_SOCKET_PROTOCOL;

typedef enum
{
	SOCK_CLOSED = 0x00,
	SOCK_ARP = 0x01,
	SOCK_INIT = 0x13,
	SOCK_LISTEN = 0x14,
	SOCK_SYNSENT = 0x15,
	SOCK_SYNRECV = 0x16,
	SOCK_ESTABLISHED = 0x17,
	SOCK_FIN_WAIT = 0x18,
	SOCK_CLOSING = 0x1A,
	SOCK_TIME_WAIT = 0x1B,
	SOCK_CLOSE_WAIT = 0x1C,
	SOCK_LAST_ACK = 0x1D,
	SOCK_UDP = 0x22,
	SOCK_IPRAW = 0x32,
	SOCK_MACRAW = 0x42,
	SOCK_PPPOE = 0x5F
}WIZNET_SOCKET_STATUS;

typedef enum
{
	WIZNET_MEM_0KB = 0x00  * 1024,
	WIZNET_MEM_1KB = 0x01  * 1024,
	WIZNET_MEM_2KB = 0x02  * 1024,
	WIZNET_MEM_4KB = 0x04  * 1024,
	WIZNET_MEM_8KB = 0x08  * 1024,
	WIZNET_MEM_16KB = 0x10 * 1024,
}WIZNET_MEM_SIZE_CHOICE;

typedef struct
{
	WIZNET_SOCKETS socket;
	WIZNET_MEM_SIZE_CHOICE rx_mem_size;
	WIZNET_MEM_SIZE_CHOICE tx_mem_size;
}WIZNET_SOCKET_MEMORY_MAP;



extern WIZNET_SOCKET_CALLBACKS WIZNET_Socket_Callback_Array[];
extern WIZNET_SOCKET_MEMORY_MAP wiznet_socket_memory_map[];

STATUS WIZNET_Module_Init(void);
STATUS WIZNET_Reset(void);
uint8_t WIZNET_GetChipVersion(void);
void WIZNET_SetIpAddress(WIZNET_IP_ADDRESS ip_address);
void WIZNET_SetSubnetMask(WIZNET_IP_ADDRESS ip_address);
WIZNET_IP_ADDRESS WIZNET_GetIpAddress(void);
WIZNET_IP_ADDRESS WIZNET_GetSubnetMask(void);
void WIZNET_SetGateway(WIZNET_IP_ADDRESS ip_address);
WIZNET_IP_ADDRESS WIZNET_GetGateway(void);
void WIZNET_SetHardwareAddress(WIZNET_HW_ADDRESS hw_address);
WIZNET_HW_ADDRESS WIZNET_GetGeneralHwAddress(uint16_t register_address); 
WIZNET_HW_ADDRESS WIZNET_GetHardwareAddress(void);
WIZNET_PHY_STATUS WIZNET_GetPhyStatus(void);
void WIZNET_SetIMR2(uint8_t value);
uint8_t WIZNET_GetIMR2(void);
uint8_t WIZNET_GetIR(void);
void WIZNET_SetIR(uint8_t value);
STATUS WIZNET_SetGeneralInterrupt(WIZNET_GENERAL_INTERRUPT_SOURCES intr_source, WIZNET_CallBack *callback);
STATUS WIZNET_ClrGeneralInterrupt(WIZNET_GENERAL_INTERRUPT_SOURCES intr_source);
uint8_t WIZNET_GetIMR(void);
void WIZNET_SetIMR(uint8_t value);
void WIZNET_SetIR2(uint8_t value);
uint8_t WIZNET_GetIR2(void);
uint8_t WIZNET_GetSocketIR(WIZNET_SOCKETS socket);
void WIZNET_SetSocketIR(WIZNET_SOCKETS socket, uint8_t value);
uint8_t WIZNET_GetSocketIMR(WIZNET_SOCKETS socket);
void WIZNET_SetSocketIMR(WIZNET_SOCKETS socket, uint8_t value);
STATUS WIZNET_SetSocketInterrupt(WIZNET_SOCKETS socket, WIZNET_SOCKET_INTERRUPT_SOURCES intr_source, WIZNET_CallBack *callback);
STATUS WIZNET_ClrSocketInterrupt(WIZNET_SOCKETS socket, WIZNET_SOCKET_INTERRUPT_SOURCES intr_source);
STATUS WIZNET_ClrAllSocketInterrupt(WIZNET_SOCKETS socket);
WIZNET_SOCKET_STATUS WIZNET_GetSocketStatus(WIZNET_SOCKETS socket);
uint16_t WIZNET_GetGeneralPort(uint16_t register_address);
void WIZNET_SetGeneralPort(uint16_t register_address, uint16_t port);
void WIZNET_SendSocketCommand(WIZNET_SOCKETS socket, WIZNET_SOCKET_COMMAND command);
void WIZNET_SetModeRegister(WIZNET_SOCKETS socket, uint8_t value);
uint8_t WIZNET_GetModeRegister(WIZNET_SOCKETS socket);
void WIZNET_SetSocketSourcePort(WIZNET_SOCKETS socket, uint16_t port);
uint16_t WIZNET_GetSocketSourcePort(WIZNET_SOCKETS socket);
WIZNET_HW_ADDRESS WIZNET_GetSocketDestinationHwAddress(WIZNET_SOCKETS socket);
void WIZNET_SetSocketDestinationHwAddress(WIZNET_SOCKETS socket, WIZNET_HW_ADDRESS hw_address);
WIZNET_IP_ADDRESS WIZNET_GetSocketDestinationIpAddress(WIZNET_SOCKETS socket);
void WIZNET_SetSocketDestinationIpAddress(WIZNET_SOCKETS socket, WIZNET_IP_ADDRESS ip_address);
STATUS WIZNET_ConfigureSocket_TCPServer(WIZNET_SOCKETS socket, uint16_t port, uint32_t keep_alive_period);
uint16_t WIZNET_GetReceivedSize(WIZNET_SOCKETS socket);
uint16_t WIZNET_GetRxReadPointer(WIZNET_SOCKETS socket);
void WIZNET_SetRxReadPointer(WIZNET_SOCKETS socket, uint16_t value);
void WIZNET_ReadRxMemory(uint16_t address, uint8_t *dest, uint16_t size);
uint16_t WIZNET_CalculateRxSocketOffset(WIZNET_SOCKETS socket);
uint16_t WIZNET_CalculateTxSocketOffset(WIZNET_SOCKETS socket);
uint32_t WIZNET_ReadData(WIZNET_SOCKETS socket, uint8_t *buffer);
void WIZNET_Process(void);
uint16_t WIZNET_GetSocketTxFreeSize(WIZNET_SOCKETS socket);
void WIZNET_SetSocketTxWritePointer(WIZNET_SOCKETS socket, uint16_t value);
uint16_t WIZNET_GetSocketTxWritePointer(WIZNET_SOCKETS socket);
void WIZNET_WriteTxMemory(uint16_t address, uint8_t *src, uint16_t size);
STATUS WIZNET_SendData(WIZNET_SOCKETS socket, uint8_t *buffer, uint32_t size);
void WIZNET_InitSocket(WIZNET_SOCKETS socket);
void WIZNET_ListenSocket(WIZNET_SOCKETS socket);
STATUS WIZNET_OpenSocket_TCPServer(WIZNET_SOCKETS socket);
void WIZNET_DisconnectSocket(WIZNET_SOCKETS socket);
void WIZNET_SetWiznetMemory(void);

#endif
