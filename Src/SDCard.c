#include "SDCard.h"
#include "board.h"

/// SPI command set
#define SD_CMD0			0	// R1: Reset
#define SD_CMD1			1	// R1: Initialize (use ACMD41 instead)
#define SD_CMD6			6	// R1: Switch function
#define SD_CMD8			8	// R7: Get Interface condition
#define SD_CMD9			9	// R1: Get Card Specific Data (CSD)
#define SD_CMD10		10	// R1: Get Card identification (CID)
#define SD_CMD12		12	// R1b: Stop transmission
#define SD_CMD13		13	// R2: Get Status
#define SD_CMD16		16	// R1: Set block length (standard capacity only)
#define SD_CMD17		17	// R1: Read single block
#define SD_CMD18		18	// R3: Read multiple blocks
#define SD_CMD24		24	// R1: Write single block
#define SD_CMD25		25	// R1: Start writing blocks
#define SD_CMD27		27	// R1: Set CSD
#define SD_CMD28		28	// R1b: Set write protect (standard capacity only)
#define SD_CMD29		29	// R1b: Clear write protect (standard capacity only)
#define SD_CMD30		30	// R1: Get write protect (standard capacity only)
#define SD_CMD32		32	// R1: Set first block to be erased
#define SD_CMD33		33	// R1: Set last block to be erased
#define SD_CMD38		38	// R1b: Erase marked block
#define SD_CMD42		42	// R1: Lock / unlock card
#define SD_CMD55		55	// R1: ACMD to be followed
#define SD_CMD56		56	// R1: Read/write data block
#define SD_CMD58		58	// R3: Read OCR register
#define SD_CMD59		59	// R1: Set CRC option
#define SD_ACMD13		13	// R2: Get SD status
#define SD_ACMD22		22	// R1: Get number of successfully written blocks
#define SD_ACMD23		23	// R1: Set number of blocks to be written
#define SD_ACMD41		41	// R1: Activates card
#define SD_ACMD51		51	// R1: Get SCR register

// alias
#define SD_CMD_GO_IDLE_STATE			SD_CMD0		// R1
#define SD_CMD_SEND_OP_COND				SD_CMD1		// R1
#define SD_CMD_SWITCH_FUNC				SD_CMD6		// R1
#define SD_CMD_SEND_IF_COND				SD_CMD8		// R7
#define SD_CMD_SEND_CSD					SD_CMD9		// R1
#define SD_CMD_SEND_CID					SD_CMD10	// R1
#define SD_CMD_STOP_TRANSMISSION		SD_CMD12	// R1b
#define SD_CMD_SEND_STATUS				SD_CMD13	// R2
#define SD_CMD_SET_BLOCKLEN				SD_CMD16	// R1
#define SD_CMD_READ_SINGLE_BLOCK		SD_CMD17	// R1
#define SD_CMD_READ_MULTIPLE_BLOCK		SD_CMD18	// R1
#define SD_CMD_WRITE_BLOCK				SD_CMD24	// R1
#define SD_CMD_WRITE_MULTIPLE_BLOCK		SD_CMD25	// R1
#define SD_CMD_PROGRAM_CSD				SD_CMD27	// R1
#define SD_CMD_ERASE_WR_BLK_START_ADDR	SD_CMD32	// R1
#define SD_CMD_ERASE_WR_BLK_END_ADDR	SD_CMD33	// R1
#define SD_CMD_ERASE					SD_CMD38	// R1b
#define SD_CMD_LOCK_UNLOCK				SD_CMD42	// 42
#define SD_CMD_APP_CMD					SD_CMD55	// R1
#define SD_CMD_GEN_CMD					SD_CMD56	// R1
#define SD_CMD_READ_OCR					SD_CMD58	// R3
#define SD_CMD_CRC_ON_OFF				SD_CMD59	// R1
#define SD_ACMD_SD_STATUS				SD_ACMD13
#define SD_ACMD_SET_WR_BLK_ERASE_COUNT	SD_ACMD23
#define SD_ACMD_SD_SEND_OP_COND			SD_ACMD41
#define SD_ACMD_SEND_SCR				SD_ACMD51

// data response
#define SD_DATA_ACCEPT					(0x05)
#define SD_DATA_CRCERR					(0x0b)
#define SD_DATA_WRTERR					(0x0d)

// data token
#define DATA_TOKEN_CMD17			0xfe
#define DATA_TOKEN_CMD18			0xfe
#define DATA_TOKEN_CMD24			0xfe
#define DATA_TOKEN_CMD25_START		0xfc
#define DATA_TOKEN_CMD25_STOP		0xfd

// card type
#define CTYPE_UNKNOWN		0
#define CTYPE_SDV1			1
#define CTYPE_MMC			2
#define CTYPE_SDV2			3
#define CTYPE_SDHC			4


static struct _cardinfo
{
	uint8_t cardType;
	uint8_t csdStruct;
	uint8_t blockSize;
	uint32_t capacityMb;
	uint8_t eraseBlkEn;
	uint8_t sectorSize;
} cardInfo;

static SDStatus sdStatus = SDCARD_NOINIT;

/// Get data response
bool SDCard_GetDataResponse(void);
/// Read CSD register
bool SDCard_ReadCSD(void);

/** Perform initialization sequence
 */
bool SDCard_Init(void)
{
	bool flag = false;
	uint8_t u8val = 0xff;
	uint8_t dummy[10] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	SDC_Response r;

	// reset high capacity flag
	cardInfo.cardType = CTYPE_UNKNOWN;

	// apply 74 or more clock with MOSI and CS high
	SDC_CS_HIGH;
	SDC_TX(dummy, 10);

	// reset the card
	SDC_CS_LOW;
	r = SDC_SendCommand(SD_CMD0, 0, 0x95, 1);
	SDC_CS_HIGH;

	// valid response of the reset received
	if(r.r1 == 0x01)
	{
		// check interface (voltage) condition
		SDC_CS_LOW;
		r = SDC_SendCommand(SD_CMD8, 0x1AA, 0x87, 5);
		SDC_CS_HIGH;

		// CMD8 1AA accepted: SD ver.2
		if(r.r1 == 0x01)
		{
			if(((r.r4<<8) + r.r5) == 0x1AA)
			{
				// set maximum delay to 1000msec
				u8val = 100;
				// initialize the card with ACMD41
				SDC_CS_LOW;
				do
				{
					SDC_SendCommand(SD_CMD_APP_CMD, 0, 0xff, 1);
					r = SDC_SendCommand(SD_ACMD41, 0x40000000, 0xff, 1);

					MSecDelay(10);
					u8val--;
				}
				while((r.r1 == 0x01) && (u8val));
				SDC_CS_HIGH;

				// card initialized
				if(r.r1 == 0x0)
				{
					// read OCR
					SDC_CS_LOW;
					r = SDC_SendCommand(SD_CMD58, 0x0, 0xff, 5);
					SDC_CS_HIGH;

					// check CCS bit of OCR
					if(r.r2 & 0x40)
					{
						// high capacity card: we are done here
						DbgPrintf("\r\nSD v2 high capacity detected");
						cardInfo.cardType = CTYPE_SDHC;
						flag = true;
					}
					else
					{
						// standard capacity card: set block size to 512 bytes
						SDC_CS_LOW;
						SDC_SendCommand(SD_CMD_SET_BLOCKLEN, 0x200, 0xff, 1);
						SDC_CS_HIGH;

						DbgPrintf("\r\nSD v2 standard capacity detected");
						cardInfo.cardType = CTYPE_SDV2;
						flag = true;
					}
				}
			}
		}
		// CMD8 1AA not accepted: SD ver.1 or MMC ver.3
		else if(r.r1 == 0x05)
		{
			// set maximum delay to 1000msec
			u8val = 100;
			// initialize the card with ACMD41
			SDC_CS_LOW;
			do
			{
				SDC_SendCommand(SD_CMD_APP_CMD, 0, 0xff, 1);
				r = SDC_SendCommand(SD_ACMD41, 0x0, 0xff, 1);

				MSecDelay(10);
				u8val--;
			}
			while((r.r1 == 0x01) && u8val);
			SDC_CS_HIGH;

			// card initialized
			if(r.r1 == 0x0)
			{
				// SD ver.1: set block size to 512 bytes
				SDC_CS_LOW;
				SDC_SendCommand(SD_CMD_SET_BLOCKLEN, 0x200, 0xff, 1);
				SDC_CS_HIGH;

				DbgPrintf("\r\nSD v1 detected");
				cardInfo.cardType = CTYPE_SDV1;
				flag = true;
			}
			// failed to initialize: try CMD1 instead
			else
			{
				// set maximum delay to 1000msec
				u8val = 100;
				// initialize the card with CMD1
				SDC_CS_LOW;
				do
				{
					r = SDC_SendCommand(SD_CMD_SEND_OP_COND, 0x0, 0xff, 1);
					MSecDelay(10);
					u8val--;
				}
				while((r.r1 == 0x01) && u8val);
				SDC_CS_HIGH;

				// card initialized
				if(r.r1 == 0x0)
				{
					// MMC v3: set block size to 512 bytes
					SDC_CS_LOW;
					SDC_SendCommand(SD_CMD_SET_BLOCKLEN, 0x200, 0xff, 1);
					SDC_CS_HIGH;

					DbgPrintf("\r\nMMC v3 detected");
					cardInfo.cardType = CTYPE_MMC;
					flag = true;
				}
			}
		}
	}

	if(flag)
	{
		if(SDCard_ReadCSD() == false)
		{
			DbgPrintf("\r\n<ERR>SDCard_ReadCSD");
			flag = false;
		}
	}

	if(flag)
	{
		sdStatus = SDCARD_READY;
	}

	return flag;
}


/** Read one block (512 bytes) of data from the card
 *
 *	\param block block number to be read
 *	\buff buffer for the data
 *	\return true data read success
 */
bool SDCard_ReadBlock(uint32_t block, uint8_t *buff)
{
	bool flag = false;
	uint8_t crc16[] = {0xff,0xff};
	SDC_Response r;

	if(cardInfo.cardType == CTYPE_UNKNOWN)
	{
		DbgPrintf("\r\n<ERR>Card type unknown");
		return false;
	}
	else if(cardInfo.cardType != CTYPE_SDHC)
	{
		// convert block number to address
		block = BLOCK_TO_ADDR(block);
	}

	// select the device
	SDC_CS_LOW;
	// Due to the oddity of STM32Cube, buffer must be filled with 0xff
	for(int i = 0; i < SD_BLOCK_SIZE; i++)
	{
		buff[i] = 0xff;
	}
	// wait until the card is idle
	if(SDC_WaitByte(0xff, 0xff))
	{
		// CMD17: READ_SINGLE_BLOCK
		r = SDC_SendCommand(SD_CMD17, block, 0xff, 1);

		// command acknowledged
		if(r.r1 == 0x00)
		{
			// waiting for the data token
			if(SDC_WaitByte(DATA_TOKEN_CMD17, 0xff))
			{
				// read out the block
				SDC_RX(buff, SD_BLOCK_SIZE);
				// receive at least two more bytes (CRC)
				SDC_RX(crc16, 2);
				// skip the CRC check
				flag = true;
			}
			else
			{
				DbgPrintf("\r\n<ERR>SDCard_ReadBlock: DTOKEN timeout");
			}
		}
		else
		{
			DbgPrintf("\r\n<ERR>SDCard_ReadBlock: r.r1(%x)",r.r1);
		}
	}
	else
	{
		DbgPrintf("\r\n<ERR>SDCard_ReadBlock: card busy");
	}
	// release the device
	SDC_CS_HIGH;

	return flag;
}

bool SDCard_WriteBlock(uint32_t block, uint8_t *buff)
{
	bool flag = false;
	uint8_t crc16[] = {0xff,0xff};
	uint8_t dtoken = DATA_TOKEN_CMD24;
	SDC_Response r;

	if(cardInfo.cardType == CTYPE_UNKNOWN)
	{
		DbgPrintf("\r\n<ERR>Card type unknown");
		return false;
	}
	else if(cardInfo.cardType != CTYPE_SDHC)
	{
		// convert block number to address
		block = BLOCK_TO_ADDR(block);
	}

	// select the device
	SDC_CS_LOW;
	// wait until the card is idle
	if(SDC_WaitByte(0xff, 0xff))
	{
		// CMD24: WRITE_BLOCK
		r = SDC_SendCommand(SD_CMD24, block, 0xff, 1);

		// command acknowledged
		if(r.r1 == 0x00)
		{
			// write at least one dummy byte
			SDC_TX(crc16, 2);
			// write data token
			SDC_TX(&dtoken, 1);
			// send the block
			SDC_TX(buff, SD_BLOCK_SIZE);
			// send at least two more bytes (CRC)
			SDC_TX(crc16, 2);
			// check data response
			if(SDCard_GetDataResponse())
			{
				// note that we do not wait for the job done
				flag = true;
			}
		}
		else
		{
			DbgPrintf("\r\n<ERR>SDCard_WriteBlock: r.r1(%x)",r.r1);
		}
	}
	else
	{
		DbgPrintf("\r\n<ERR>SDCard_WriteBlock: card busy");
	}
	// release the device
	SDC_CS_HIGH;

	return flag;
}


bool SDCard_GetDataResponse(void)
{
	uint8_t txb[2] = {0xff,0xff};
	uint8_t rxb[2];
	bool flag = false;

	// receive data response and provide 8 more clocks
	SDC_TXRX(txb,rxb,2);

	if((rxb[0] & 0x1f) == SD_DATA_ACCEPT)
	{
		flag = true;
	}
	else
	{
		DbgPrintf("\r\n<ERR>Data Response:%x", rxb[0]);
	}

	return flag;
}


bool SDCard_ReadCSD(void)
{
	bool flag = false;
	uint8_t buff[16] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
						0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
					   };
	uint8_t crc16[2] = {0xff,0xff};
	uint32_t u32val1,u32val2;
	SDC_Response r;

	// select the device
	SDC_CS_LOW;

	// wait until the card is idle
	if(SDC_WaitByte(0xff, 0xffff))
	{
		// CMD9: SEND_CSD
		r = SDC_SendCommand(SD_CMD9, 0x0, 0xff, 1);

		// command acknowledged
		if(r.r1 == 0x00)
		{
			// waiting for the data token
			if(SDC_WaitByte(DATA_TOKEN_CMD17, 0xffff) == true)
			{
				// read out the register value
				SDC_RX(buff, 16);
				// receive two more bytes (CRC)
				SDC_RX(crc16, 2);

				// fill necessary information
				cardInfo.csdStruct = (buff[0] & 0xc0)>>6;
				cardInfo.blockSize = 2<<(buff[5] & 0x0f);
				// CSD ver.1
				if(cardInfo.csdStruct == 0)
				{
					// csize
					u32val1 = ((buff[6] & 0x03)<<10) + (buff[7]<<2) +
							  ((buff[8] & 0xC0)>>6);
					// csize_mult
					u32val2 = ((buff[9] & 0x03)<<1) + ((buff[10] & 0x80) >> 7);
					// memory capacity in MB
					cardInfo.capacityMb = (u32val1 +1) * (2<<(u32val2+2));

					if(cardInfo.blockSize == 512)
					{
						cardInfo.capacityMb = cardInfo.capacityMb>>1;
					}
					else if(cardInfo.blockSize == 2048)
					{
						cardInfo.capacityMb = cardInfo.capacityMb<<1;
					}
				}
				// CSD ver.2
				else
				{
					u32val1 = ((buff[7] & 0x3F)<<16) + (buff[8]<<8) + buff[9];
					cardInfo.capacityMb = (u32val1+1)>>1 ;
				}
				cardInfo.eraseBlkEn = (buff[10] & 0x40)>>6;
				cardInfo.sectorSize = ((buff[10] & 0x3f)<<1) +
									  ((buff[11] & 0x80)<<7);
				flag = true;
			}
		}
		else
		{
			DbgPrintf("\r\n<ERR>SDCard_ReadCSD: r.r1(%x)",r.r1);
		}
	}
	else
	{
		DbgPrintf("\r\n<ERR>SDCard_ReadCSD: card busy");
	}

	SDC_CS_HIGH;

	return flag;
}


SDStatus SDCard_GetStatus(void)
{
	return sdStatus;
}

void SDCard_SetStatus(SDStatus status)
{
	sdStatus = status;
}

uint32_t SDCard_GetCapacityMB(void)
{
	return cardInfo.capacityMb;
}

bool SDCard_Sync(void)
{
	return( SDC_WaitByte(0xff, 0xffff) );
}

#if UNIT_TEST

#define MAX_BLOCK_NUMBER	5
void SDCard_UnitTest(void)
{
	bool flag = true;
	int i;
	uint8_t buff[SD_BLOCK_SIZE];

	// print out cardInfo
	DbgPrintf("\r\n\r\nCard Type: %d", cardInfo.cardType);
	DbgPrintf("\r\nCSD Struct: %d", cardInfo.csdStruct);
	DbgPrintf("\r\nBlock Size: %d", cardInfo.blockSize);
	DbgPrintf("\r\nCapacity(MB): %d", cardInfo.capacityMb);
	DbgPrintf("\r\nErase Blk: %d", cardInfo.eraseBlkEn);
	DbgPrintf("\r\nSector Size: %d", cardInfo.sectorSize);

	// fill the buffer with some data
	i = 0;
	while(i < SD_BLOCK_SIZE)
	{
		buff[i] = (uint8_t)(i+128);
		i++;
	}

#if 0
	DbgPrintf("\r\nSDCard_WriteBlock....");
	i = 0;
	while(i < MAX_BLOCK_NUMBER)
	{
		DbgPrintf("%d.",i);
		if(SDCard_WriteBlock(i, buff) == false)
		{
			DbgPrintf("\r\nSDCard_WriteBlock failed");
			break;
		}
		i++;
	}
#endif
	DbgPrintf("\r\nSDCard_ReadBlock....");
	i = 0;
	while(i < MAX_BLOCK_NUMBER)
	{
		DbgPrintf("%d.",i);
		if(SDCard_ReadBlock(i, buff) == false)
		{
			DbgPrintf("\r\nSDCard_ReadBlock failed");
			//flag = false;
			//break;
		}
		i++;
	}

	if(flag)
	{
		// print out the current buffer contents
		i = 0;
		DbgPrintf("\r\n---------------------------------------\r\n");
		while(i < SD_BLOCK_SIZE)
		{
			if((i % 16) == 0)
			{
				DbgPrintf("(%04d) ", i);
			}

			DbgPrintf("%02X", buff[i++]);

			if((i % 16) == 0)
			{
				DbgPrintf("\r\n");
			}
			else if((i % 8) == 0)
			{
				DbgPrintf(" - ");
			}
			else
			{
				DbgPrintf(".");
			}
		}
	}
}

#endif // UNIT_TEST
