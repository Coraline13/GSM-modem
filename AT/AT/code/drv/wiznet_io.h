#ifndef __WIZNET_IO_H
#define __WIZNET_IO_H

#include <stdint.h>

void WIZNET_Init_IO(void);
void WIZNET_Select(void);
void WIZNET_Deselect(void);

void WIZNET_ResetLo(void);
void WIZNET_ResetHi(void);

uint16_t WIZNET_SPI_SendReceive(uint16_t data);
void WIZNET_InitInterrupt(void);
uint8_t WIZNET_InterruptPinLow(void);
#endif
