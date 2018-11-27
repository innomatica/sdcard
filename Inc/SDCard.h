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

#define ADDR_TO_BLOCK(x)				((x)>>9)
#define BLOCK_TO_ADDR(x)				((x)<<9)

#define SD_BLOCK_SIZE					(512)

typedef enum
{
	SDCARD_READY,
	SDCARD_NOINIT,
	SDCARD_NODISK
} SDStatus;;

/// Initialize device
bool SDCard_Init(void);
/// Read blocks
bool SDCard_ReadBlock(uint32_t block, uint8_t *buff);
/// Write blocks
bool SDCard_WriteBlock(uint32_t block, uint8_t *buff);
/// Get status
SDStatus  SDCard_GetStatus(void);
/// Set status
void SDCard_SetStatus(SDStatus status);
/// Get capacity in MB
uint32_t SDCard_GetCapacityMB(void);
/// Sync data
bool SDCard_Sync(void);
/// Unit test
void SDCard_UnitTest(void);


#endif // __SDCARD_H
