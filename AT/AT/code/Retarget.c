/*----------------------------------------------------------------------------
 * Name:    Retarget.c
 * Purpose: 'Retarget' layer for target-dependent low level functions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2011 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <rt_misc.h>
#include <stdint.h>
#include <DRV\drv_general.h>
#include <DRV\drv_uart.h>
#include "retarget.h"
#pragma import(__use_no_semihosting_swi)



struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

void initRetargetDebugSystem()
{
	DRV_UART_Configure(DEBUG_UART, UART_CHARACTER_LENGTH_8, 115200, UART_PARITY_NO_PARITY, 1, TRUE, 3);
}

int fputc(int c, FILE *f) {
  return (DRV_UART_SendByte(DEBUG_UART, c));
}


int fgetc(FILE *f) {
	uint8_t ch;
	DRV_UART_ReadByte(DEBUG_UART, &ch);
	return (ch);
}


int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int c) {
  DRV_UART_SendByte(DEBUG_UART, c);
}


void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}
