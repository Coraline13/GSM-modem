#include "wiznet_io.h"
#include "drv_general.h"
#include <stdint.h>
//#include <Hal/spi.h>


void WIZNET_Init_IO()
{
	//slave select
/*
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	*/
	// reset
	/*
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStruct);	
	DRV_SPI_Init(SPI_1);
	*/
	WIZNET_Deselect();
	WIZNET_ResetHi();
}

void WIZNET_InitInterrupt()
{/*
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource6);

	EXTI_InitStructure.EXTI_Line = EXTI_Line6;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0xF;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0xF;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
	*/
}

void WIZNET_Select()
{
//	GPIO_ResetBits(GPIOC, GPIO_Pin_4);
}

void WIZNET_Deselect()
{
//	GPIO_SetBits(GPIOC, GPIO_Pin_4);
}

void WIZNET_ResetLo()
{
//	GPIO_ResetBits(GPIOC, GPIO_Pin_5);
}

void WIZNET_ResetHi()
{
//	GPIO_SetBits(GPIOC, GPIO_Pin_5);
}

uint16_t WIZNET_SPI_SendReceive(uint16_t data)
{
	return 0;

//	return DRV_SPI_SendReceive(SPI_1, data);
}

uint8_t WIZNET_InterruptPinLow()
{
	return 0;
//	return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6);
}
