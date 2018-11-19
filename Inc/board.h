#ifndef __BOARD_H
#define __BOARD_H
/*
 * \author Sungjune Lee
 *
 */
#include <stdint.h>
#include <stdbool.h>
#include "stm32l0xx_hal.h"

#define LD1_ON					(LD1_GPIO_Port->BSRR = LD1_Pin)
#define LD1_OFF					(LD1_GPIO_Port->BRR = LD1_Pin)
#define LD1_TOGGLE				(LD1_GPIO_Port->ODR ^= LD1_Pin)
#define LD2_ON					(LD2_GPIO_Port->BSRR = LD2_Pin)
#define LD2_OFF					(LD2_GPIO_Port->BRR = LD2_Pin)
#define LD2_TOGGLE				(LD2_GPIO_Port->ODR ^= LD2_Pin)
// LED
#define LED_ON					(LD1_GPIO_Port->BSRR = LD1_Pin)
#define LED_OFF					(LD1_GPIO_Port->BRR = LD1_Pin)
#define LED_TOGGLE				(LD1_GPIO_Port->ODR ^= LD1_Pin)

// BTN
#define BTN_READ				(((BTN_GPIO_Port->IDR)&(BTN_Pin))==(BTN_Pin)?(0):(1))

// Amplifier
#define AMP_ON					(AMP_EN_GPIO_Port->BSRR = AMP_EN_Pin)
#define AMP_OFF					(AMP_EN_GPIO_Port->BRR = AMP_EN_Pin)

// Accelerometer
#define ACCEL_CS_LOW			(ACC_CS_GPIO_Port->BRR = ACC_CS_Pin)
#define ACCEL_CS_HIGH			(ACC_CS_GPIO_Port->BSRR = ACC_CS_Pin)

// SD card
#define SDC_CS_LOW				(SDC_EN_GPIO_Port->BRR = SDC_EN_Pin)
#define SDC_CS_HIGH				(SDC_EN_GPIO_Port->BSRR = SDC_EN_Pin)
#define SDC_TIMEOUT				(500)
#define SDC_TX(a,n)				do{\
								HAL_SPI_Transmit(&hspi1,a,n,SDC_TIMEOUT);\
								}while(0)
#define SDC_RX(a,n)				do{\
								HAL_SPI_Receive(&hspi1,a,n,SDC_TIMEOUT);\
								}while(0)
#define SDC_TXRX(a,b,n)			do{\
								HAL_SPI_TransmitReceive(&hspi1,a,b,n,SDC_TIMEOUT);\
								}while(0)

// Serial Flash
#define MEM_CS_LOW				(MEM_CS_GPIO_Port->BRR = MEM_CS_Pin)
#define MEM_CS_HIGH				(MEM_CS_GPIO_Port->BSRR = MEM_CS_Pin)
#define MEM_TIMEOUT				(500)
#define MEM_TX(a,n)				do{\
								HAL_SPI_Transmit(&hspi1,a,n,MEM_TIMEOUT);\
								}while(0)
#define MEM_RX(a,n)				do{\
								HAL_SPI_Receive(&hspi1,a,n,MEM_TIMEOUT);\
								}while(0)
#define MEM_TXRX(a,b,n)			do{\
								HAL_SPI_TransmitReceive(&hspi1,a,b,n,MEM_TIMEOUT);\
								}while(0)
#define MEM_NUM_SECTORS			(0x100)

// SysTick interrupt control (LL_SYStICK_EnableIT/LL_SYSTICK_DisableIT)
#define SYSTICK_ENABLE_IT		do{\
								SET_BIT(SysTick->CTRL,SysTick_CTRL_TICKINT_Msk);\
								}while(0)
#define SYSTICK_DISABLE_IT		do{\
								CLEAR_BIT(SysTick->CTRL,SysTick_CTRL_TICKINT_Msk);\
								}while(0)
// USART interrupt control (LL_USART_EnableIT_RXNE/LL_USART_DisableIT_RXNE);
#define USART1_ENABLE_RX_IT		do{\
								SET_BIT(USART1->CR1, USART_CR1_RXNEIE);\
								}while(0)
#define USART1_DISABLE_RX_IT	do{\
								CLEAR_BIT(USART1->CR1, USART_CR1_RXNEIE);\
								}while(0)
// LL_USART_RequestRxDataFlush
#define USART1_CLEAR_RX_IT		do{\
								SET_BIT(USART1->RQR, USART_RQR_RXFRQ);\
								}while(0)
#define CLEAR_UART_RX_IT		USART1_CLEAR_RX_IT
#define USART1_GET_DATA			((uint8_t) READ_REG(USART1->RDR))

// ByteQueue Critical Section
#define BQUEUE_ENTER_CS			USART1_DISABLE_RX_IT
#define BQUEUE_EXIT_CS			USART1_ENABLE_RX_IT

// debug
#define DEBUG_OUTPUT			(1)
#define UNIT_TEST				(0)

#if DEBUG_OUTPUT
#define DbgPrintf(x,arg...)		UART_Printf(x,##arg)
#else
#define DbgPrintf(x,arg...)		{}
#endif

#define	SAMPLE_FREQ				(16000)

typedef struct
{
	uint8_t r1;
	uint8_t r2;
	uint8_t r3;
	uint8_t r4;
	uint8_t r5;
} SDC_Response;

#define SDC_CMD_LEN				(6)
#define MSecDelay(x)			do{HAL_Delay(x);}while(0);

extern SPI_HandleTypeDef hspi1;

/// General purpose functions
void USB_Printf(const char* format,...);
void UART_Printf(const char* format,...);
void USecDelay(unsigned usec);

void SerialComm_Init(void);
void SerialComm_SendByteArray(uint8_t *buffer, int size);

SDC_Response SDC_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc, uint8_t rlen);
bool SDC_WaitByte(uint8_t byte, uint16_t timeout);

#endif	// __BOARD_H
