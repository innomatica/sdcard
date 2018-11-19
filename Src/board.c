#include <stdarg.h>
#include <stdio.h>
#include "board.h"

extern UART_HandleTypeDef huart1;
/** Formatted string output to USB_CDC
 *
 *	\param printf() like parameters
#include "usbd_cdc.h"
extern USBD_HandleTypeDef hUsbDeviceFS;
void USB_Printf(const char* format,...)
{
	char buffer[256];
	int length;
	va_list args;
	va_start(args, format);

	length = vsprintf(buffer, format, args);
	if(length)
		USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*)buffer, length);

	va_end(args);
}
 */


/** Formatted string ouput to UART
 *
 *	\param printf() like parameters
 */
void UART_Printf(const char* format,...)
{
	char buffer[256];
	int length;
	va_list args;
	va_start(args, format);

	length = vsprintf(buffer, format, args);
	if(length)
		HAL_UART_Transmit(&huart1, (uint8_t*)buffer, length, 1000);

	va_end(args);
}


/** Microsecond delay
 *
 * \warning This is subject to error since the delay relies on execution of
 *		nop().
 */
void USecDelay(unsigned usec)
{
	while(usec-- > 0)
	{
		// approximately 1 usec delay in 32MHz clock
		asm(
			"nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
			"nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
			"nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
		);
	}
}

uint8_t PushButton_Read(void)
{
	return BTN_READ;
}

void SerialComm_Init(void)
{
	// clear USART interrupt
	HAL_NVIC_ClearPendingIRQ(USART1_IRQn);
	// enable RX interrupt
	USART1_ENABLE_RX_IT;
}

void SerialComm_SendByteArray(uint8_t *buffer, int size)
{
	// note that HAL_UART_TransmitIT no longer works
	HAL_UART_Transmit(&huart1, buffer, size, 1000);
}

/** Sending dummy (0xff) data while waiting for the target byte to be arrived.
 *
 *	\param byte target byte
 *	\param timeout
 */
bool SDC_WaitByte(uint8_t byte, uint16_t timeout)
{
	uint8_t txb = 0xff, rxb;

	while(1)
	{
		// send 0xff, read byte
		SDC_TXRX(&txb, &rxb, 1);

		// target byte received or timeout occurred
		if((rxb == byte) || (timeout == 0))
		{
			break;
		}
		else
		{
			timeout--;
		}
	}

	// target byte received
	if(rxb == byte)
	{
		return true;
	}
	// timeout occurred
	else
	{
		return false;
	}
}


SDC_Response SDC_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc, uint8_t rlen)
{
	uint8_t txb[SDC_CMD_LEN], rxb[SDC_CMD_LEN];
	// Ncr 8 bytes
	uint8_t timeout = 0x08;

	SDC_Response r = {0xff,0xff,0xff,0xff,0xff};

	// command byte
	txb[0] = cmd | 0x40;
	// payload word
	txb[1] = (uint8_t)(arg >> 24);
	txb[2] = (uint8_t)(arg >> 16);
	txb[3] = (uint8_t)(arg >> 8);
	txb[4] = (uint8_t)(arg >> 0);
	// checksum byte
	txb[5] =  crc | 0x01;

	// transmit command packet
	SDC_TX(txb, SDC_CMD_LEN);

	// clear tx buffer and rx buffer
	for(int i = 0; i < rlen; i++)
	{
		txb[i] = rxb[i] = 0xff;
	}
	// response must be received within Ncr
	do
	{
		SDC_TXRX(txb, rxb, 1);
		timeout--;
	}
	while((rxb[0] == 0xff) && timeout);

	if(timeout != 0)
	{
		// store the value
		r.r1 = rxb[0];
		// remaining response length
		rlen--;

		// need more bytes to receive
		if(rlen)
		{
			// get the rest of the respnse
			SDC_TXRX(txb, rxb, rlen);
			// store them in the structure
			r.r2 = rxb[0];
			r.r3 = rxb[1];
			r.r4 = rxb[2];
			r.r5 = rxb[3];
		}
	}
	else
	{
		DbgPrintf("\r\n<ERR>SDC_SendCommand timeout");
	}

	return r;
}

#if UNIT_TEST

#endif
