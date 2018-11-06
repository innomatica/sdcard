#ifndef __SDCARD_H
#define __SDCARD_H
/**
 * 	\file
 *	\author	<a href="http://www.innomatic.ca">innomatic</a>
 *	\brief	SD card SPI interface
 * 	\copyright <a rel="license" href="http://creativecommons.org/licenses/by-nc/4.0/"><img alt="Creative Commons Licence" style="border-width:0" src="https://i.creativecommons.org/l/by-nc/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc/4.0/">Creative Commons Attribution-NonCommercial 4.0 International License</a>.
 *
 *	\warning If there is no device in the system or if the device is not
 *	functional then any of the write functions may hang the system.
 */

#include <stdbool.h>
#include <stdint.h>
#include "board.h"

/// SDCard command set
#define SD_CMD_GO_IDLE_STATE			0
#define SD_CMD_SEND_OP_COND				1
#define SD_CMD_SWITCH_FUNC				6
#define SD_CMD_SEND_IF_COND				8
#define SD_CMD_SEND_CSD					9
#define SD_CMD_SEND_CID					10
#define SD_CMD_STOP_TRANSMISSION		12
#define SD_CMD_SEND_STATUS				13
#define SD_CMD_SET_BLOCKLEN				16		// only for CMD42
#define SD_CMD_READ_SINGLE_BLOCK		17
#define SD_CMD_READ_MULTIPLE_BLOCK		18
#define SD_CMD_WRITE_BLOCK				24
#define SD_CMD_WRITE_MULTIPLE_BLOCK		25
#define SD_CMD_PROGRAM_CSD				27
#define SD_CMD_ERASE_WR_BLK_START_ADDR	32
#define SD_CMD_ERASE_WR_BLK_END_ADDR	33
#define SD_CMD_ERASE					38
#define SD_CMD_LOCK_UNLOCK				42
#define SD_CMD_APP_CMD					55
#define SD_CMD_GEN_CMD					56
#define SD_CMD_READ_OCR					58
#define SD_CMD_CRC_ON_OFF				59
#define SD_ACMD_SD_STATUS				13
#define SD_ACMD_SET_WR_BLK_ERASE_COUNT	23
#define SD_ACMD_SD_SEND_OP_COND			41
#define SD_ACMD_SEND_SCR				51

/// Initialize device
void SDCard_Init(void);
/// Check if the device is busy
bool SDCard_IsBusy(void);
/// Checi if the device is writable
bool SDCard_IsWritable(void);
/// Read the status byte
uint8_t SDCard_ReadStatus(void);
/// Erase one sector
void SDCard_EraseSector(uint32_t sector);
/// Read data
void SDCard_Read(uint32_t addr, uint8_t *buff, uint16_t len);
/// Write data
void SDCard_Write(uint32_t addr, uint8_t *buff, uint16_t len) __attribute((weak));
/// Read data by sector unit
void SDCard_ReadSector(uint32_t sector, uint8_t *buff);
/// Write data by sector unit
void SDCard_WriteSector(uint32_t sector, uint8_t *buff);
/// Unit test
void SDCard_UnitTest(void);

#endif // __SDCARD_H
