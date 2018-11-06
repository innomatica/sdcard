#include "SDCard.h"
#include "board.h"

typedef struct
{
	uint8_t r1;
	uint8_t r2;
	uint8_t r3;
	uint8_t r4;
	uint8_t r5;
} SD_Response;

#define SD_CMD_LEN	(6)

SD_Response SD_SendCommand(uint8_t cmd, uint32_t arg)
{
	uint8_t txb[SD_CMD_LEN], rxb[SD_CMD_LEN];
	uint8_t rlen, timeout = 0x08;
	SD_Response r = {0xff,0xff,0xff,0xff,0xff};

	txb[0] = cmd | 0x40;
	txb[1] = (uint8_t)(arg >> 24);
	txb[2] = (uint8_t)(arg >> 16);
	txb[3] = (uint8_t)(arg >> 8);
	txb[4] = (uint8_t)(arg >> 0);
	txb[5] =  0x01;

	// response len;
	switch(cmd)
	{
	// R1b
	case SD_CMD_STOP_TRANSMISSION:
		rlen = 2;
		break;
	// R2
	case SD_CMD_SEND_STATUS:
		rlen = 2;
		break;
	// R3
	case SD_CMD_READ_OCR:
	// R7
	case SD_CMD_SEND_IF_COND:
		rlen = 5;
		break;
	// R1
	default:
		rlen = 1;
		break;
	}


	SDCARD_CS_LOW;

	// transmit command
	//
	SDCARD_TX(txb, SD_CMD_LEN);
	for(int i = 0; i < rlen; i++)
	{
		txb[i] = rxb[i] = 0xff;
	}

	// get first byte of the response
	do
	{
		SDCARD_TXRX(txb, rxb, 1);
		timeout--;
	}
	while((rxb[0] == 0xff) && timeout);
	// store the value
	r.r1 = rxb[0];
	rlen--;
	// get the rest bytes of the response
	if(rlen)
	{
		SDCARD_TXRX(txb, rxb, rlen);
		r.r2 = rxb[0];
		r.r3 = rxb[1];
		r.r4 = rxb[2];
		r.r5 = rxb[3];
	}


	SDCARD_CS_HIGH;

	return r;
}

void SDCard_Init(void)
{
	SDCARD_CS_HIGH;
	uint8_t dummy = 0xff;

	for(int i = 0; i < 10; i++)
	{
		SDCARD_TX(&dummy, 1);
	}
}
